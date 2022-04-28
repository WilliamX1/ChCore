/*
 * Copyright (c) 2022 Institute of Parallel And Distributed Systems (IPADS)
 * ChCore-Lab is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include <machine.h>
#include <object/object.h>
#include <object/cap_group.h>
#include <object/thread.h>
#include <mm/kmalloc.h>
#include <mm/uaccess.h>
#include <lib/printk.h>

extern void pmo_deinit(void *);
extern void connection_deinit(void *);
extern void vmspace_deinit(void *);
extern void cap_group_deinit(void *);

const obj_deinit_func obj_deinit_tbl[TYPE_NR] = {
        [0 ... TYPE_NR - 1] = NULL,
        [TYPE_CAP_GROUP] = cap_group_deinit,
        [TYPE_THREAD] = thread_deinit,
        [TYPE_CONNECTION] = connection_deinit,
        [TYPE_PMO] = pmo_deinit,
        [TYPE_VMSPACE] = vmspace_deinit,
};

/*
 * Usage:
 * obj = obj_alloc(...);
 * initialize the obj;
 * cap_alloc(obj);
 */
void *obj_alloc(u64 type, u64 size)
{
        u64 total_size;
        struct object *object;

        total_size = sizeof(*object) + size;
        object = kzalloc(total_size);
        if (!object)
                return NULL;

        object->type = type;
        object->size = size;
        object->refcount = 0;

        /*
         * If the cap of the object is copied, then the copied cap (slot) is
         * stored in such a list.
         */
        init_list_head(&object->copies_head);

        return object->opaque;
}

/*
 * After the fail initialization of a cap (after obj_alloc and before
 * cap_alloc), invoke this interface to free the object allocated by obj_alloc.
 */
void obj_free(void *obj)
{
        struct object *object;

        if (!obj)
                return;
        object = container_of(obj, struct object, opaque);

        BUG_ON(object->refcount != 0);
        kfree(object);
}

int cap_alloc(struct cap_group *cap_group, void *obj, u64 rights)
{
        struct object *object;
        struct object_slot *slot;
        int r, slot_id;

        object = container_of(obj, struct object, opaque);

        slot_id = alloc_slot_id(cap_group);
        if (slot_id < 0) {
                r = -ENOMEM;
                goto out_table;
        }

        slot = kmalloc(sizeof(*slot));
        if (!slot) {
                r = -ENOMEM;
                goto out_free_slot_id;
        }
        slot->slot_id = slot_id;
        slot->cap_group = cap_group;
        slot->isvalid = true;
        slot->rights = rights;
        slot->object = object;
        list_add(&slot->copies, &object->copies_head);

        BUG_ON(object->refcount != 0);
        object->refcount = 1;

        install_slot(cap_group, slot_id, slot);

        return slot_id;
out_free_slot_id:
        free_slot_id(cap_group, slot_id);
out_table:
        return r;
}

/* An internal interface: only invoked by __cap_free and obj_put. */
void __free_object(struct object *object)
{
#ifndef TEST_OBJECT
        obj_deinit_func func;

        /* Invoke the object-specific free routine */
        func = obj_deinit_tbl[object->type];
        if (func)
                func(object->opaque);
#endif

        BUG_ON(!list_empty(&object->copies_head));
        kfree(object);
}

/*
 * cap_free (__cap_free) only removes one cap, which differs from cap_free_all.
 */
int __cap_free(struct cap_group *cap_group, int slot_id)
{
        struct object_slot *slot;
        struct object *object;
        int r = 0;
        u64 old_refcount;

        /* Step-1: free the slot_id (i.e., the capability number) in the slot
         * table */
        slot = get_slot(cap_group, slot_id);
        if (!slot || slot->isvalid == false) {
                r = -ECAPBILITY;
                goto out_table;
        }

        free_slot_id(cap_group, slot_id);

        /* Step-2: remove the slot in the copies-list of the object and free the
         * slot */
        object = slot->object;
        list_del(&slot->copies);
        kfree(slot);

        /* Step-3: decrease the refcnt of the object and free it if necessary */
        old_refcount = atomic_fetch_sub_64(&object->refcount, 1);

        if (old_refcount == 1)
                __free_object(object);

        return 0;

out_table:
        return r;
}

int cap_free(struct cap_group *cap_group, int slot_id)
{
        return __cap_free(cap_group, slot_id);
}

int cap_copy(struct cap_group *src_cap_group, struct cap_group *dest_cap_group,
             int src_slot_id)
{
        struct object_slot *src_slot, *dest_slot;
        int r, dest_slot_id;

        src_slot = get_slot(src_cap_group, src_slot_id);
        if (!src_slot || src_slot->isvalid == false) {
                r = -ECAPBILITY;
                goto out;
        }

        dest_slot_id = alloc_slot_id(dest_cap_group);
        if (dest_slot_id == -1) {
                r = -ENOMEM;
                goto out;
        }

        dest_slot = kmalloc(sizeof(*dest_slot));
        if (!dest_slot) {
                r = -ENOMEM;
                goto out_free_slot_id;
        }
        src_slot = get_slot(src_cap_group, src_slot_id);
        atomic_fetch_add_64(&src_slot->object->refcount, 1);

        dest_slot->slot_id = dest_slot_id;
        dest_slot->cap_group = dest_cap_group;
        dest_slot->isvalid = true;
        dest_slot->object = src_slot->object;

        list_add(&dest_slot->copies, &src_slot->copies);

        install_slot(dest_cap_group, dest_slot_id, dest_slot);
        return dest_slot_id;
out_free_slot_id:
        free_slot_id(dest_cap_group, dest_slot_id);
out:
        return r;
}

int cap_move(struct cap_group *src_cap_group, struct cap_group *dest_cap_group,
             int src_slot_id)
{
        int r;

        r = cap_copy(src_cap_group, dest_cap_group, src_slot_id);
        if (r < 0)
                return r;

        r = cap_free(src_cap_group, src_slot_id);
        BUG_ON(r); /* if copied successfully, free should not fail */

        return r;
}

/*
 * Free an object points by some cap, which also removes all the caps point to
 * the object.
 */
int cap_free_all(struct cap_group *cap_group, int slot_id)
{
        void *obj;
        struct object *object;
        struct object_slot *slot_iter = NULL, *slot_iter_tmp = NULL;
        int r;

        /*
         * Since obj_get requires to pass the cap type
         * which is not available here, get_opaque is used instead.
         */
        obj = get_opaque(cap_group, slot_id, false, 0);

        if (!obj) {
                r = -ECAPBILITY;
                goto out_fail;
        }

        object = container_of(obj, struct object, opaque);

        /* free all copied slots */
        for_each_in_list_safe (
                slot_iter, slot_iter_tmp, copies, &object->copies_head) {
                u64 iter_slot_id = slot_iter->slot_id;
                struct cap_group *iter_cap_group = slot_iter->cap_group;

                r = __cap_free(iter_cap_group, iter_slot_id);
                BUG_ON(r != 0);
        }

        /* get_opaque will also add the reference cnt */
        obj_put(obj);

        return 0;

out_fail:
        return r;
}

int sys_cap_copy_to(u64 dest_cap_group_cap, u64 src_slot_id)
{
        struct cap_group *dest_cap_group;
        int r;

        dest_cap_group =
                obj_get(current_cap_group, dest_cap_group_cap, TYPE_CAP_GROUP);
        if (!dest_cap_group)
                return -ECAPBILITY;
        r = cap_copy(current_cap_group, dest_cap_group, src_slot_id);
        obj_put(dest_cap_group);
        return r;
}

int sys_cap_copy_from(u64 src_cap_group_cap, u64 src_slot_id)
{
        struct cap_group *src_cap_group;
        int r;

        src_cap_group =
                obj_get(current_cap_group, src_cap_group_cap, TYPE_CAP_GROUP);
        if (!src_cap_group)
                return -ECAPBILITY;
        r = cap_copy(src_cap_group, current_cap_group, src_slot_id);
        obj_put(src_cap_group);
        return r;
}

int sys_transfer_caps(u64 dest_group_cap, u64 src_caps_buf, int nr_caps,
                      u64 dst_caps_buf)
{
        struct cap_group *dest_cap_group;
        int i;
        int *src_caps;
        int *dst_caps;
        size_t size;

        dest_cap_group =
                obj_get(current_cap_group, dest_group_cap, TYPE_CAP_GROUP);
        if (!dest_cap_group)
                return -ECAPBILITY;

        size = sizeof(int) * nr_caps;
        src_caps = kmalloc(size);
        dst_caps = kmalloc(size);

        /* get args from user buffer */
        copy_from_user((void *)src_caps, (void *)src_caps_buf, size);

        for (i = 0; i < nr_caps; ++i) {
                dst_caps[i] = cap_copy(
                        current_cap_group, dest_cap_group, src_caps[i]);
        }

        /* write results to user buffer */
        copy_to_user((void *)dst_caps_buf, (void *)dst_caps, size);

        kfree(src_caps);
        kfree(dst_caps);

        obj_put(dest_cap_group);
        return 0;
}

int sys_cap_move(u64 dest_cap_group_cap, u64 src_slot_id)
{
        struct cap_group *dest_cap_group;
        int r;

        dest_cap_group =
                obj_get(current_cap_group, dest_cap_group_cap, TYPE_CAP_GROUP);
        if (!dest_cap_group)
                return -ECAPBILITY;
        r = cap_move(current_cap_group, dest_cap_group, src_slot_id);
        obj_put(dest_cap_group);
        return r;
}

// for debug
int sys_get_all_caps(u64 cap_group_cap)
{
        struct cap_group *cap_group;
        struct slot_table *slot_table;
        int i;

        cap_group = obj_get(current_cap_group, cap_group_cap, TYPE_CAP_GROUP);
        if (!cap_group)
                return -ECAPBILITY;
        printk("thread %p cap:\n", current_thread);

        slot_table = &cap_group->slot_table;
        for (i = 0; i < slot_table->slots_size; i++) {
                struct object_slot *slot = get_slot(cap_group, i);
                if (!slot)
                        continue;
                BUG_ON(slot->isvalid != true);
                printk("slot_id:%d type:%d\n",
                       i,
                       slot_table->slots[i]->object->type);
        }

        obj_put(cap_group);
        return 0;
}

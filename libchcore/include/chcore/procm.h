#pragma once

#define PROCM_PATH_LEN 256
#define PROCM_ARGC_MAX 256

#ifdef __cplusplus
extern "C" {
#endif

enum procm_ipc_request {
        PROCM_IPC_REQ_SPAWN,
        PROCM_IPC_REQ_MAX,
};

struct procm_ipc_data {
        enum procm_ipc_request request;
        union {
                struct {
                        struct {
                                char path[PROCM_PATH_LEN];
                        } args;
                        struct {
                                int pid;
                        } returns;
                } spawn;
        };
};

/**
 * Spawn a new process via procm.
 * @param path Path to the executable file.
 * @param mt_cap_out Capability of the main thread of the spawned process.
 */
int chcore_procm_spawn(const char *path, int *mt_cap_out);

#ifdef __cplusplus
}
#endif

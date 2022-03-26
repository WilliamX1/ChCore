#pragma once

#define le16toh(x) (x)
#define le32toh(x) (x)
#define le64toh(x) (x)

#define be16toh(x) ((((x)&0xff) << 8) | (((x) >> 8) & 0xff))
#define be32toh(x) ((be16toh((x)) << 16) | (be16toh((x) >> 16)))
#define be64toh(x) ((be32toh((x)) << 32) | (be32toh((x) >> 32)))

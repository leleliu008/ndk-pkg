#include "core/sysinfo.h"
#include "ndk-pkg.h"

int ndkpkg_sysinfo() {
    SysInfo sysinfo = {0};

    int ret = sysinfo_make(&sysinfo);

    if (ret == NDKPKG_OK) {
        sysinfo_dump(&sysinfo);
        sysinfo_free(&sysinfo);
    }

    return ret;
}

#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "core/log.h"
#include "core/tar.h"

#include "ndk-pkg.h"

int ndkpkg_pack(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, ArchiveType outputType, const char * outputPath, const bool verbose) {
    if (targetPlatform == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    NDKPKGReceipt * receipt = NULL;

    int ret = ndkpkg_receipt_parse(packageName, targetPlatform, &receipt);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    size_t packingDIRNameCapacity = strlen(packageName) + strlen(receipt->version) + strlen(receipt->builtFor) + 3U;
    char   packingDIRName[packingDIRNameCapacity];

    ret = snprintf(packingDIRName, packingDIRNameCapacity, "%s-%s-%s", packageName, receipt->version, receipt->builtFor);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ndkpkg_receipt_free(receipt);

    /////////////////////////////////////////////////////////////////////////////////

    const char * outputFileExt;

    switch (outputType) {
        case ArchiveType_tar_gz:  outputFileExt = ".tar.gz";  break;
        case ArchiveType_tar_xz:  outputFileExt = ".tar.xz";  break;
        case ArchiveType_tar_lz:  outputFileExt = ".tar.lz";  break;
        case ArchiveType_tar_bz2: outputFileExt = ".tar.bz2"; break;
        case ArchiveType_zip:     outputFileExt = ".zip";     break;
    }

    /////////////////////////////////////////////////////////////////////////////////

    char buf[PATH_MAX];

    char * cwd = getcwd(buf, PATH_MAX);

    if (cwd == NULL) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////

    char outputFilePath[PATH_MAX];

    if (outputPath == NULL) {
        ret = snprintf(outputFilePath, PATH_MAX, "%s/%s%s", cwd, packingDIRName, outputFileExt);
    } else {
        size_t outputPathLength = strlen(outputPath);

        if (outputPath[outputPathLength - 1U] == '/') {
            if (outputPath[0] == '/') {
                ret = snprintf(outputFilePath, PATH_MAX, "%s%s%s", outputPath, packingDIRName, outputFileExt);
            } else {
                ret = snprintf(outputFilePath, PATH_MAX, "%s/%s%s%s", cwd, outputPath, packingDIRName, outputFileExt);
            }
        } else {
            if (outputPath[0] == '/') {
                strncpy(outputFilePath, outputPath, outputPathLength);
                outputFilePath[outputPathLength] = '\0';
            } else {
                ret = snprintf(outputFilePath, PATH_MAX, "%s/%s", cwd, outputPath);
            }
        }
    }

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    char   sessionDIR[PATH_MAX] = {0};
    size_t sessionDIRLength;

    ret = ndkpkg_session_dir(sessionDIR, PATH_MAX, &sessionDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    /////////////////////////////////////////////////////////////////////////////////

    if (chdir(sessionDIR) != 0) {
        perror(sessionDIR);
        return NDKPKG_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////

    const char * targetABI;

    switch (targetPlatform->abi) {
        case AndroidABI_arm64_v8a:
            targetABI = "arm64-v8a";
            break;
        case AndroidABI_armeabi_v7a:
            targetABI = "armeabi-v7a";
            break;
        case AndroidABI_x86_64:
            targetABI = "x86_64";
            break;
        case AndroidABI_x86:
            targetABI = "x86";
            break;
    }

    size_t packageInstalledDIRCapacity = ndkpkgHomeDIRLength + strlen(packageName) + 33U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/installed/android-%2u-%s/%s", ndkpkgHomeDIR, targetPlatform->api, targetABI, packageName);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    if (symlink(packageInstalledDIR, packingDIRName) != 0) {
        perror (packageInstalledDIR);
        return NDKPKG_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////

    size_t tmpFilePathCapacity = sessionDIRLength + packingDIRNameCapacity + 10U;
    char   tmpFilePath[tmpFilePathCapacity];

    ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/%s%s", sessionDIR, packingDIRName, outputFileExt);

    if (ret < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    ret = tar_create(packingDIRName, tmpFilePath, outputType, verbose);

    if (ret != 0) {
        return ret;
    }

    ret = ndkpkg_rename_or_copy_file(tmpFilePath, outputFilePath);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    /////////////////////////////////////////////////////////////////////////////////

    struct stat st;

    if (stat(outputFilePath, &st) != 0) {
        perror(outputFilePath);
        return NDKPKG_ERROR;
    }

    off_t nBytes = st.st_size;

    if (nBytes < 1024) {
        printf("%s %ld Byte\n", outputFilePath, nBytes);
    } else if (nBytes < 1024 * 1024) {
        printf("%s %.2f KB\n", outputFilePath, nBytes / 1024.0);
    } else if (nBytes < 1024 * 1024 * 1024) {
        printf("%s %.2f MB\n", outputFilePath, nBytes / 1024.0 / 1024.0);
    } else {
        LOG_ERROR2("file is too large: ", tmpFilePath);
        return NDKPKG_ERROR;
    }

    return ndkpkg_rm_r(sessionDIR, verbose);
}

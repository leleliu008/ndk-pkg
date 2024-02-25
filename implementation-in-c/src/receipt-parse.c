#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <limits.h>

#include <yaml.h>

#include "core/regex/regex.h"

#include "ndk-pkg.h"

typedef enum {
    NDKPKGReceiptKeyCode_unknown,

    NDKPKGReceiptKeyCode_summary,
    NDKPKGReceiptKeyCode_version,
    NDKPKGReceiptKeyCode_license,

    NDKPKGReceiptKeyCode_web_url,

    NDKPKGReceiptKeyCode_git_url,
    NDKPKGReceiptKeyCode_git_sha,
    NDKPKGReceiptKeyCode_git_ref,
    NDKPKGReceiptKeyCode_git_nth,

    NDKPKGReceiptKeyCode_src_url,
    NDKPKGReceiptKeyCode_src_uri,
    NDKPKGReceiptKeyCode_src_sha,

    NDKPKGReceiptKeyCode_fix_url,
    NDKPKGReceiptKeyCode_fix_uri,
    NDKPKGReceiptKeyCode_fix_sha,

    NDKPKGReceiptKeyCode_res_url,
    NDKPKGReceiptKeyCode_res_uri,
    NDKPKGReceiptKeyCode_res_sha,

    NDKPKGReceiptKeyCode_dep_pkg,
    NDKPKGReceiptKeyCode_dep_upp,
    NDKPKGReceiptKeyCode_dep_pym,
    NDKPKGReceiptKeyCode_dep_plm,

    NDKPKGReceiptKeyCode_bsystem,
    NDKPKGReceiptKeyCode_bscript,
    NDKPKGReceiptKeyCode_binbstd,

    NDKPKGReceiptKeyCode_do12345,
    NDKPKGReceiptKeyCode_dopatch,
    NDKPKGReceiptKeyCode_install,
    NDKPKGReceiptKeyCode_symlink,

    NDKPKGReceiptKeyCode_ppflags,
    NDKPKGReceiptKeyCode_ccflags,
    NDKPKGReceiptKeyCode_xxflags,
    NDKPKGReceiptKeyCode_ldflags,

    NDKPKGReceiptKeyCode_parallel,

    NDKPKGReceiptKeyCode_builtby,
    NDKPKGReceiptKeyCode_builtat,
    NDKPKGReceiptKeyCode_builtfor,
} NDKPKGReceiptKeyCode;

void ndkpkg_receipt_dump(NDKPKGReceipt * receipt) {
    if (receipt == NULL) {
        return;
    }

    printf("summary: %s\n", receipt->summary);
    printf("version: %s\n", receipt->version);
    printf("license: %s\n", receipt->license);

    printf("web-url: %s\n", receipt->web_url);

    printf("git_url: %s\n", receipt->git_url);
    printf("git_sha: %s\n", receipt->git_sha);
    printf("git_ref: %s\n", receipt->git_ref);
    printf("git_nth: %zu\n", receipt->git_nth);

    printf("src_url: %s\n", receipt->src_url);
    printf("src_uri: %s\n", receipt->src_uri);
    printf("src_sha: %s\n", receipt->src_sha);

    printf("fix_url: %s\n", receipt->fix_url);
    printf("fix_uri: %s\n", receipt->fix_uri);
    printf("fix_sha: %s\n", receipt->fix_sha);

    printf("res_url: %s\n", receipt->res_url);
    printf("res_uri: %s\n", receipt->res_uri);
    printf("res_sha: %s\n", receipt->res_sha);

    printf("dep_pkg: %s\n", receipt->dep_pkg);
    printf("dep_upp: %s\n", receipt->dep_upp);
    printf("dep_pym: %s\n", receipt->dep_pym);
    printf("dep_plm: %s\n", receipt->dep_plm);

    printf("bsystem: %s\n", receipt->bsystem);
    printf("bscript: %s\n", receipt->bscript);
    printf("binbstd: %d\n", receipt->binbstd);
    printf("parallel: %d\n", receipt->parallel);

    printf("do12345: %s\n", receipt->dopatch);
    printf("dopatch: %s\n", receipt->dopatch);
    printf("install: %s\n", receipt->install);

    printf("symlink: %d\n", receipt->symlink);

    printf("builtby: %s\n", receipt->builtBy);
    printf("builtat: %s\n", receipt->builtAt);
    printf("builtfor: %s\n", receipt->builtFor);

    printf("path:    %s\n", receipt->path);
}

void ndkpkg_receipt_free(NDKPKGReceipt * receipt) {
    if (receipt == NULL) {
        return;
    }

    if (receipt->summary != NULL) {
        free(receipt->summary);
        receipt->summary = NULL;
    }

    if (receipt->version != NULL) {
        free(receipt->version);
        receipt->version = NULL;
    }

    if (receipt->license != NULL) {
        free(receipt->license);
        receipt->license = NULL;
    }

    if (receipt->web_url != NULL) {
        free(receipt->web_url);
        receipt->web_url = NULL;
    }

    ///////////////////////////////

    if (receipt->git_url != NULL) {
        free(receipt->git_url);
        receipt->git_url = NULL;
    }

    if (receipt->git_sha != NULL) {
        free(receipt->git_sha);
        receipt->git_sha = NULL;
    }

    if (receipt->git_ref != NULL) {
        free(receipt->git_ref);
        receipt->git_ref = NULL;
    }

    ///////////////////////////////

    if (receipt->src_url != NULL) {
        free(receipt->src_url);
        receipt->src_url = NULL;
    }

    if (receipt->src_uri != NULL) {
        free(receipt->src_uri);
        receipt->src_uri = NULL;
    }

    if (receipt->src_sha != NULL) {
        free(receipt->src_sha);
        receipt->src_sha = NULL;
    }

    ///////////////////////////////

    if (receipt->fix_url != NULL) {
        free(receipt->fix_url);
        receipt->fix_url = NULL;
    }

    if (receipt->fix_uri != NULL) {
        free(receipt->fix_uri);
        receipt->fix_uri = NULL;
    }

    if (receipt->fix_sha != NULL) {
        free(receipt->fix_sha);
        receipt->fix_sha = NULL;
    }

    ///////////////////////////////

    if (receipt->res_url != NULL) {
        free(receipt->res_url);
        receipt->res_url = NULL;
    }

    if (receipt->res_uri != NULL) {
        free(receipt->res_uri);
        receipt->res_uri = NULL;
    }

    if (receipt->res_sha != NULL) {
        free(receipt->res_sha);
        receipt->res_sha = NULL;
    }

    ///////////////////////////////

    if (receipt->dep_pkg != NULL) {
        free(receipt->dep_pkg);
        receipt->dep_pkg = NULL;
    }

    if (receipt->dep_upp != NULL) {
        free(receipt->dep_upp);
        receipt->dep_upp = NULL;
    }

    if (receipt->dep_pym != NULL) {
        free(receipt->dep_pym);
        receipt->dep_pym = NULL;
    }

    if (receipt->dep_plm != NULL) {
        free(receipt->dep_plm);
        receipt->dep_plm = NULL;
    }

    ///////////////////////////////

    if (receipt->bsystem != NULL) {
        free(receipt->bsystem);
        receipt->bsystem = NULL;
    }

    if (receipt->bscript != NULL) {
        free(receipt->bscript);
        receipt->bscript = NULL;
    }

    ///////////////////////////////

    if (receipt->ppflags != NULL) {
        free(receipt->ppflags);
        receipt->ppflags = NULL;
    }

    if (receipt->ccflags != NULL) {
        free(receipt->ccflags);
        receipt->ccflags = NULL;
    }

    if (receipt->xxflags != NULL) {
        free(receipt->xxflags);
        receipt->xxflags = NULL;
    }

    if (receipt->ldflags != NULL) {
        free(receipt->ldflags);
        receipt->ldflags = NULL;
    }

    ///////////////////////////////

    if (receipt->do12345 != NULL) {
        free(receipt->do12345);
        receipt->do12345 = NULL;
    }

    if (receipt->dopatch != NULL) {
        free(receipt->dopatch);
        receipt->dopatch = NULL;
    }

    if (receipt->install != NULL) {
        free(receipt->install);
        receipt->install = NULL;
    }

    ///////////////////////////////

    if (receipt->path != NULL) {
        free(receipt->path);
        receipt->path = NULL;
    }

    ///////////////////////////////

    if (receipt->builtBy != NULL) {
        free(receipt->builtBy);
        receipt->builtBy = NULL;
    }

    if (receipt->builtAt != NULL) {
        free(receipt->builtAt);
        receipt->builtAt = NULL;
    }

    if (receipt->builtFor != NULL) {
        free(receipt->builtFor);
        receipt->builtAt = NULL;
    }

    free(receipt);
}

static NDKPKGReceiptKeyCode ndkpkg_receipt_key_code_from_key_name(char * key) {
           if (strcmp(key, "summary") == 0) {
        return NDKPKGReceiptKeyCode_summary;
    } else if (strcmp(key, "version") == 0) {
        return NDKPKGReceiptKeyCode_version;
    } else if (strcmp(key, "license") == 0) {
        return NDKPKGReceiptKeyCode_license;
    } else if (strcmp(key, "web-url") == 0) {
        return NDKPKGReceiptKeyCode_web_url;
    } else if (strcmp(key, "git-url") == 0) {
        return NDKPKGReceiptKeyCode_git_url;
    } else if (strcmp(key, "git-sha") == 0) {
        return NDKPKGReceiptKeyCode_git_sha;
    } else if (strcmp(key, "git-ref") == 0) {
        return NDKPKGReceiptKeyCode_git_ref;
    } else if (strcmp(key, "git-nth") == 0) {
        return NDKPKGReceiptKeyCode_git_nth;
    } else if (strcmp(key, "src-url") == 0) {
        return NDKPKGReceiptKeyCode_src_url;
    } else if (strcmp(key, "src-uri") == 0) {
        return NDKPKGReceiptKeyCode_src_uri;
    } else if (strcmp(key, "src-sha") == 0) {
        return NDKPKGReceiptKeyCode_src_sha;
    } else if (strcmp(key, "fix-url") == 0) {
        return NDKPKGReceiptKeyCode_fix_url;
    } else if (strcmp(key, "fix-uri") == 0) {
        return NDKPKGReceiptKeyCode_fix_uri;
    } else if (strcmp(key, "fix-sha") == 0) {
        return NDKPKGReceiptKeyCode_fix_sha;
    } else if (strcmp(key, "res-url") == 0) {
        return NDKPKGReceiptKeyCode_res_url;
    } else if (strcmp(key, "res-uri") == 0) {
        return NDKPKGReceiptKeyCode_res_uri;
    } else if (strcmp(key, "res-sha") == 0) {
        return NDKPKGReceiptKeyCode_res_sha;
    } else if (strcmp(key, "dep-pkg") == 0) {
        return NDKPKGReceiptKeyCode_dep_pkg;
    } else if (strcmp(key, "dep-upp") == 0) {
        return NDKPKGReceiptKeyCode_dep_upp;
    } else if (strcmp(key, "dep-pym") == 0) {
        return NDKPKGReceiptKeyCode_dep_pym;
    } else if (strcmp(key, "dep-plm") == 0) {
        return NDKPKGReceiptKeyCode_dep_plm;
    } else if (strcmp(key, "ppflags") == 0) {
        return NDKPKGReceiptKeyCode_ppflags;
    } else if (strcmp(key, "ccflags") == 0) {
        return NDKPKGReceiptKeyCode_ccflags;
    } else if (strcmp(key, "xxflags") == 0) {
        return NDKPKGReceiptKeyCode_xxflags;
    } else if (strcmp(key, "ldflags") == 0) {
        return NDKPKGReceiptKeyCode_ldflags;
    } else if (strcmp(key, "do12345") == 0) {
        return NDKPKGReceiptKeyCode_do12345;
    } else if (strcmp(key, "dopatch") == 0) {
        return NDKPKGReceiptKeyCode_dopatch;
    } else if (strcmp(key, "install") == 0) {
        return NDKPKGReceiptKeyCode_install;
    } else if (strcmp(key, "symlink") == 0) {
        return NDKPKGReceiptKeyCode_symlink;
    } else if (strcmp(key, "bsystem") == 0) {
        return NDKPKGReceiptKeyCode_bsystem;
    } else if (strcmp(key, "bscript") == 0) {
        return NDKPKGReceiptKeyCode_bscript;
    } else if (strcmp(key, "binbstd") == 0) {
        return NDKPKGReceiptKeyCode_binbstd;
    } else if (strcmp(key, "parallel") == 0) {
        return NDKPKGReceiptKeyCode_parallel;
    } else if (strcmp(key, "builtby") == 0) {
        return NDKPKGReceiptKeyCode_builtby;
    } else if (strcmp(key, "builtat") == 0) {
        return NDKPKGReceiptKeyCode_builtat;
    } else if (strcmp(key, "builtfor") == 0) {
        return NDKPKGReceiptKeyCode_builtfor;
    } else {
        return NDKPKGReceiptKeyCode_unknown;
    }
}

static void ndkpkg_receipt_set_value(NDKPKGReceiptKeyCode keyCode, char * value, NDKPKGReceipt * receipt) {
    if (keyCode == NDKPKGReceiptKeyCode_unknown) {
        return;
    }

    for (;;) {
        char c = value[0];

        if (c == '\0') {
            return;
        }

        // non-printable ASCII characters and space
        if (c <= 32) {
            value++;
        } else {
            break;
        }
    }

    switch (keyCode) {
        case NDKPKGReceiptKeyCode_summary: if (receipt->summary != NULL) free(receipt->summary); receipt->summary = strdup(value); break;
        case NDKPKGReceiptKeyCode_version: if (receipt->version != NULL) free(receipt->version); receipt->version = strdup(value); break;
        case NDKPKGReceiptKeyCode_license: if (receipt->license != NULL) free(receipt->license); receipt->license = strdup(value); break;

        case NDKPKGReceiptKeyCode_web_url: if (receipt->web_url != NULL) free(receipt->web_url); receipt->web_url = strdup(value); break;

        case NDKPKGReceiptKeyCode_git_url: if (receipt->git_url != NULL) free(receipt->git_url); receipt->git_url = strdup(value); break;
        case NDKPKGReceiptKeyCode_git_sha: if (receipt->git_sha != NULL) free(receipt->git_sha); receipt->git_sha = strdup(value); break;
        case NDKPKGReceiptKeyCode_git_ref: if (receipt->git_ref != NULL) free(receipt->git_ref); receipt->git_ref = strdup(value); break;

        case NDKPKGReceiptKeyCode_src_url: if (receipt->src_url != NULL) free(receipt->src_url); receipt->src_url = strdup(value); break;
        case NDKPKGReceiptKeyCode_src_uri: if (receipt->src_uri != NULL) free(receipt->src_uri); receipt->src_uri = strdup(value); break;
        case NDKPKGReceiptKeyCode_src_sha: if (receipt->src_sha != NULL) free(receipt->src_sha); receipt->src_sha = strdup(value); break;

        case NDKPKGReceiptKeyCode_fix_url: if (receipt->fix_url != NULL) free(receipt->fix_url); receipt->fix_url = strdup(value); break;
        case NDKPKGReceiptKeyCode_fix_uri: if (receipt->fix_uri != NULL) free(receipt->fix_uri); receipt->fix_uri = strdup(value); break;
        case NDKPKGReceiptKeyCode_fix_sha: if (receipt->fix_sha != NULL) free(receipt->fix_sha); receipt->fix_sha = strdup(value); break;

        case NDKPKGReceiptKeyCode_res_url: if (receipt->res_url != NULL) free(receipt->res_url); receipt->res_url = strdup(value); break;
        case NDKPKGReceiptKeyCode_res_uri: if (receipt->res_uri != NULL) free(receipt->res_uri); receipt->res_uri = strdup(value); break;
        case NDKPKGReceiptKeyCode_res_sha: if (receipt->res_sha != NULL) free(receipt->res_sha); receipt->res_sha = strdup(value); break;

        case NDKPKGReceiptKeyCode_dep_pkg: if (receipt->dep_pkg != NULL) free(receipt->dep_pkg); receipt->dep_pkg = strdup(value); break;
        case NDKPKGReceiptKeyCode_dep_upp: if (receipt->dep_upp != NULL) free(receipt->dep_upp); receipt->dep_upp = strdup(value); break;
        case NDKPKGReceiptKeyCode_dep_pym: if (receipt->dep_pym != NULL) free(receipt->dep_pym); receipt->dep_pym = strdup(value); break;
        case NDKPKGReceiptKeyCode_dep_plm: if (receipt->dep_plm != NULL) free(receipt->dep_plm); receipt->dep_plm = strdup(value); break;

        case NDKPKGReceiptKeyCode_ppflags: if (receipt->ppflags != NULL) free(receipt->ppflags); receipt->ppflags = strdup(value); break;
        case NDKPKGReceiptKeyCode_ccflags: if (receipt->ccflags != NULL) free(receipt->ccflags); receipt->ccflags = strdup(value); break;
        case NDKPKGReceiptKeyCode_xxflags: if (receipt->xxflags != NULL) free(receipt->xxflags); receipt->xxflags = strdup(value); break;
        case NDKPKGReceiptKeyCode_ldflags: if (receipt->ldflags != NULL) free(receipt->ldflags); receipt->ldflags = strdup(value); break;

        case NDKPKGReceiptKeyCode_do12345: if (receipt->do12345 != NULL) free(receipt->do12345); receipt->do12345 = strdup(value); break;
        case NDKPKGReceiptKeyCode_dopatch: if (receipt->dopatch != NULL) free(receipt->dopatch); receipt->dopatch = strdup(value); break;
        case NDKPKGReceiptKeyCode_install: if (receipt->install != NULL) free(receipt->install); receipt->install = strdup(value); break;

        case NDKPKGReceiptKeyCode_bsystem: if (receipt->bsystem != NULL) free(receipt->bsystem); receipt->bsystem = strdup(value); break;
        case NDKPKGReceiptKeyCode_bscript: if (receipt->bscript != NULL) free(receipt->bscript); receipt->bscript = strdup(value); break;

        case NDKPKGReceiptKeyCode_builtby: if (receipt->builtBy != NULL) free(receipt->builtBy); receipt->builtBy = strdup(value); break;
        case NDKPKGReceiptKeyCode_builtat: if (receipt->builtAt != NULL) free(receipt->builtAt); receipt->builtAt = strdup(value); break;
        case NDKPKGReceiptKeyCode_builtfor: if (receipt->builtFor != NULL) free(receipt->builtFor); receipt->builtFor = strdup(value); break;

        case NDKPKGReceiptKeyCode_git_nth:
            receipt->git_nth = atoi(value);
            break;

        case NDKPKGReceiptKeyCode_binbstd:
            if (strcmp(value, "1") == 0) {
                receipt->binbstd = true;
            }
            break;

        case NDKPKGReceiptKeyCode_symlink:
            if (strcmp(value, "1") == 0) {
                receipt->symlink = true;
            }
            break;

        case NDKPKGReceiptKeyCode_parallel:
            if (strcmp(value, "1") == 0) {
                receipt->parallel = true;
            }
            break;

        default: break;
    }
}

static int ndkpkg_receipt_check(NDKPKGReceipt * receipt, const char * receiptFilePath) {
    if (receipt->summary == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : summary mapping not found.\n", receiptFilePath);
        return NDKPKG_ERROR_FORMULA_SCHEME;
    }

    if (receipt->version == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : version mapping not found.\n", receiptFilePath);
        return NDKPKG_ERROR_FORMULA_SCHEME;
    }

    if (receipt->web_url == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : web-url mapping not found.\n", receiptFilePath);
        return NDKPKG_ERROR_FORMULA_SCHEME;
    }

    if (receipt->src_url == NULL) {
        if (receipt->git_url == NULL) {
            fprintf(stderr, "scheme error in receipt file: %s : neither src-url nor git-url mapping not found.\n", receiptFilePath);
            return NDKPKG_ERROR_FORMULA_SCHEME;
        }
    } else {
        if (strncmp(receipt->src_url, "dir://", 6) == 0) {
            ;
        } else {
            if (receipt->src_sha == NULL) {
                fprintf(stderr, "scheme error in receipt file: %s : src-sha mapping not found.\n", receiptFilePath);
                return NDKPKG_ERROR_FORMULA_SCHEME;
            }

            if (strlen(receipt->src_sha) != 64) {
                fprintf(stderr, "scheme error in receipt file: %s : src-sha mapping's value's length must be 64.\n", receiptFilePath);
                return NDKPKG_ERROR_FORMULA_SCHEME;
            }
        }
    }

    if (receipt->bsystem == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : bsystem mapping not found.\n", receiptFilePath);
        return NDKPKG_ERROR_FORMULA_SCHEME;
    }

    if (receipt->builtBy == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : builtby mapping not found.\n", receiptFilePath);
        return NDKPKG_ERROR_RECEIPT_SCHEME;
    }

    if (receipt->builtFor == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : builtfor mapping not found.\n", receiptFilePath);
        return NDKPKG_ERROR_RECEIPT_SCHEME;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (receipt->builtAt == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : builtat mapping not found.\n", receiptFilePath);
        return NDKPKG_ERROR_RECEIPT_SCHEME;
    }

    size_t i = 0;

    for (;; i++) {
        char c = receipt->builtAt[i];

        if (c == '\0') {
            break;
        }

        if ((c < '0') || (c > '9')) {
            fprintf(stderr, "scheme error in receipt file: %s : builtAt mapping's value should only contains non-numeric characters.\n", receiptFilePath);
            return NDKPKG_ERROR_RECEIPT_SCHEME;
        }
    }

    if (i != 10U) {
        fprintf(stderr, "scheme error in receipt file: %s : builtAt mapping's value's length must be 10.\n", receiptFilePath);
        return NDKPKG_ERROR_RECEIPT_SCHEME;
    }

    return NDKPKG_OK;
}

int ndkpkg_receipt_parse(const char * packageName, const NDKPKGTargetPlatform * targetPlatform, NDKPKGReceipt * * out) {
    int ret = ndkpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    if (targetPlatform == NULL) {
        return NDKPKG_ERROR_ARG_IS_NULL;
    }

    char   ndkpkgHomeDIR[PATH_MAX];
    size_t ndkpkgHomeDIRLength;

    ret = ndkpkg_home_dir(ndkpkgHomeDIR, PATH_MAX, &ndkpkgHomeDIRLength);

    if (ret != NDKPKG_OK) {
        return ret;
    }

    size_t receiptFilePathLength = ndkpkgHomeDIRLength + strlen(packageName) + 56U;
    char * receiptFilePath = (char*)calloc(receiptFilePathLength, sizeof(char));

    if (receiptFilePath == NULL) {
        return NDKPKG_ERROR_MEMORY_ALLOCATE;
    }

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

    if (snprintf(receiptFilePath, receiptFilePathLength, "%s/installed/android-%2u-%s/%s/.ndk-pkg/RECEIPT.yml", ndkpkgHomeDIR, targetPlatform->api, targetABI, packageName) < 0) {
        perror(NULL);
        return NDKPKG_ERROR;
    }

    FILE * file = fopen(receiptFilePath, "r");

    if (file == NULL) {
        perror(receiptFilePath);
        free(receiptFilePath);
        return NDKPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    yaml_parser_t parser;
    yaml_token_t  token;

    // https://libyaml.docsforge.com/master/api/yaml_parser_initialize/
    if (yaml_parser_initialize(&parser) == 0) {
        perror("Failed to initialize yaml parser");
        free(receiptFilePath);
        return NDKPKG_ERROR;
    }

    yaml_parser_set_input_file(&parser, file);

    NDKPKGReceiptKeyCode receiptKeyCode = NDKPKGReceiptKeyCode_unknown;

    NDKPKGReceipt * receipt = NULL;

    int lastTokenType = 0;

    do {
        // https://libyaml.docsforge.com/master/api/yaml_parser_scan/
        if (yaml_parser_scan(&parser, &token) == 0) {
            fprintf(stderr, "syntax error in receipt file: %s\n", receiptFilePath);
            ret = NDKPKG_ERROR_RECEIPT_SYNTAX;
            goto finalize;
        }

        switch(token.type) {
            case YAML_KEY_TOKEN:
                lastTokenType = 1;
                break;
            case YAML_VALUE_TOKEN:
                lastTokenType = 2;
                break;
            case YAML_SCALAR_TOKEN:
                if (lastTokenType == 1) {
                    receiptKeyCode = ndkpkg_receipt_key_code_from_key_name((char*)token.data.scalar.value);
                } else if (lastTokenType == 2) {
                    if (receipt == NULL) {
                        receipt = (NDKPKGReceipt*)calloc(1, sizeof(NDKPKGReceipt));

                        if (receipt == NULL) {
                            ret = NDKPKG_ERROR_MEMORY_ALLOCATE;
                            goto finalize;
                        }

                        receipt->path = receiptFilePath;
                        receipt->git_nth = 1U;
                        receipt->symlink = true;
                        receipt->binbstd = false;
                        receipt->parallel = true;
                    }

                    ndkpkg_receipt_set_value(receiptKeyCode, (char*)token.data.scalar.value, receipt);
                }
                break;
            default: 
                lastTokenType = 0;
                break;
        }

        if (token.type != YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
        }
    } while(token.type != YAML_STREAM_END_TOKEN);

finalize:
    yaml_token_delete(&token);

    yaml_parser_delete(&parser);

    fclose(file);

    // ndkpkg_receipt_dump(receipt);

    if (ret == NDKPKG_OK) {
        ret = ndkpkg_receipt_check(receipt, receiptFilePath);

        if (ret == NDKPKG_OK) {
            (*out) = receipt;
            return NDKPKG_OK;
        }
    }

    if (receipt == NULL) {
        free(receiptFilePath);
    } else {
        ndkpkg_receipt_free(receipt);
    }

    return ret;
}

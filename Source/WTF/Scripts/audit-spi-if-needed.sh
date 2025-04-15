#!/bin/sh
set -e

program="$(dirname $(dirname $(dirname "${SRCROOT}")))/${WK_ADDITIONAL_SCRIPTS_DIR}/audit-spi"
[ -f "${program}" ] || program="$(dirname $(dirname "${SRCROOT}"))/Tools/Scripts/audit-spi"

# Xcode doesn't expose the name of the discovered dependency file, but by convention, it is
# the same basename as the timestamp output.
depfile="${SCRIPT_OUTPUT_FILE_0/%.timestamp/.d}"

if [[ "${WK_AUDIT_SPI}" == YES && -f "${program}" ]]; then
    mkdir -p "${OBJROOT}/WebKitSDKDBs"
    for arch in ${ARCHS}; do
        (set -x && "${program}" \
         --sdkdb-dir "${WK_SDKDB_DIR}" \
         --sdkdb-cache "${OBJROOT}/WebKitSDKDBs/${SDK_NAME}.sqlite3" \
         --sdk-dir "${SDKROOT}" --arch-name "${arch}" \
         --depfile "${depfile}" \
         --primary-file "${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH}" $@)
     done
else
    [ -f "${program}" ] || echo "audit-spi not available, skipping" >&2
    echo "dependencies: " > "${depfile}"
fi
touch "${SCRIPT_OUTPUT_FILE_0}"

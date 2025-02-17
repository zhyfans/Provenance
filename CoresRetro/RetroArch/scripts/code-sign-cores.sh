#!/bin/sh

# Parse command line arguments
SIGN_DYLIBS=true
BUNDLE_ID_PREFIX="org.provenance-emu"

for arg in "$@"
do
    case $arg in
        -no-dylib)
        SIGN_DYLIBS=false
        shift
        ;;
        --org-identifier=*)
        NEW_PREFIX="${arg#*=}"
        if [ -n "$NEW_PREFIX" ]; then
            BUNDLE_ID_PREFIX="$NEW_PREFIX"
        fi
        shift
        ;;
    esac
done

# WARNING: You may have to run Clean in Xcode after changing CODE_SIGN_IDENTITY!

# Verify that $CODE_SIGN_IDENTITY is set
if [ -z "${CODE_SIGN_IDENTITY}" ] ; then
    echo "Warning: CODE_SIGN_IDENTITY needs to be set for code-signing!"

    if [ "${CONFIGURATION}" = "Release" ] ; then
        exit 0
    else
        # Code-signing is optional for non-release builds.
        exit 0
    fi
fi

if [ "${CODE_SIGNING_ALLOWED}" = "NO" ] ; then
    exit 0
fi

ITEMS=""
CORES_DIR="${SRCROOT}/CoresRetro/RetroArch/modules/"
echo "Cores dir: ${CORES_DIR}"
if [ -d "$CORES_DIR" ] ; then
    if [ "$SIGN_DYLIBS" = true ] ; then
        CORES=$(find "${CORES_DIR}" -depth -type d -name "*.framework" -or -name "*.dylib" -or -name "*.bundle" | sed -e "s/\(.*framework\)//")
    else
        CORES=$(find "${CORES_DIR}/FrameworksRetro" -depth -type d -name "*.framework" -or -name "*.bundle" | sed -e "s/\(.*framework\)//")
    fi
    RESULT=$?
    if [ "$RESULT" != 0 ] ; then
        exit 1
    fi

    ITEMS="${CORES}"
fi

# Prefer the expanded name, if available.
CODE_SIGN_IDENTITY_FOR_ITEMS="${EXPANDED_CODE_SIGN_IDENTITY_NAME}"
if [ "${CODE_SIGN_IDENTITY_FOR_ITEMS}" = "" ] ; then
    # Fall back to old behavior.
    CODE_SIGN_IDENTITY_FOR_ITEMS="${CODE_SIGN_IDENTITY}"
fi

echo "Identity:"
echo "${CODE_SIGN_IDENTITY_FOR_ITEMS}"

echo "Found:"
echo "${ITEMS}"

# Change the Internal Field Separator (IFS) so that spaces in paths will not cause problems below.
SAVED_IFS=$IFS
# Doing IFS=$(echo -en "\n") does not work on Xcode 10 for some reason
IFS="
"

# Loop through all items.
for ITEM in $ITEMS;
do
    if codesign --display -r- "${ITEM}" | grep -q "${CODE_SIGN_IDENTITY_FOR_ITEMS}" ; then
        echo "Skipping '${ITEM}', already signed"
    else
        echo "Signing '${ITEM}'"
        # Remove file extension from ITEM for the identifier
        ITEM_WITHOUT_EXT=$(basename "${ITEM%.*}")
        echo "Signing with Identifier: ${BUNDLE_ID_PREFIX}.${ITEM_WITHOUT_EXT}"

        codesign --force --verbose --sign "${CODE_SIGN_IDENTITY_FOR_ITEMS}" --verbose --identifier "${BUNDLE_ID_PREFIX}.${ITEM_WITHOUT_EXT}" "${ITEM}"
        RESULT=$?
        if [ "$RESULT" != 0 ] ; then
            echo "Error: Failed to sign '${ITEM}'."
            IFS=$SAVED_IFS
            exit 1
        fi
    fi
done

# Restore $IFS.
IFS=$SAVED_IFS

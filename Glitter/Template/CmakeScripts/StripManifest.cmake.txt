file(READ "${INPUT_FILE}" RAW_JSON)

string(JSON HAS_DEV ERROR_VARIABLE JSON_ERR GET "${RAW_JSON}" "development")
if(NOT JSON_ERR)
    string(JSON CLEAN_JSON REMOVE "${RAW_JSON}" "development")
else()
    set(CLEAN_JSON "${RAW_JSON}")
endif()

file(WRITE "${OUTPUT_FILE}" "${CLEAN_JSON}")
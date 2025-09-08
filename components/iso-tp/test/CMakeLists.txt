# idf_component_get_property(test_comp ${COMPONENT_NAME} "NAME")

# set(TEST_COMPONENTS ${test_comp})

idf_component_register(SRCS "tests.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES unity )

                    #    TEST_COMPONENTS ${TEST_COMPONENTS})
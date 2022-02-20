set(component_srcs "nRF905.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS ".")

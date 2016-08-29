############ Tests ##############

# HWControler test unit
add_executable(HWControler_test
	../hwcontroler.cpp
	../audiocd.cpp
	hwcontroler_test.cpp
)

# AudioCD test unit
add_executable(AudioCD_test
	../audiocd.cpp
	audiocd_test.cpp
)


# libraries needed for HWControler tests
target_link_libraries(HWControler_test
    Qt5::Test
    Qt5::Widgets
    KF5::Solid
    ${PHONON_LIBRARIES}
)


# libraries needed for AudioCD tests
target_link_libraries(AudioCD_test
    Qt5::Test
    Qt5::Widgets
    KF5::Solid
    ${PHONON_LIBRARIES}
)


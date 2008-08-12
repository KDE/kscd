############ Tests ##############

# HWControler test unit
kde4_add_unit_test(HWControler_test
	../hwcontroler.cpp
	../audiocd.cpp
	hwcontroler_test.cpp
)

# AudioCD test unit
kde4_add_unit_test(AudioCD_test
	../audiocd.cpp
	audiocd_test.cpp
)


# libraries needed for HWControler tests
target_link_libraries(HWControler_test
	${KDE4_KUTILS_LIBS}
	${QT_QTTEST_LIBRARY}
	kcddb
	kcompactdisc
)


# libraries needed for AudioCD tests
target_link_libraries(AudioCD_test
	${KDE4_KUTILS_LIBS}
	${QT_QTTEST_LIBRARY}
	kcddb
	kcompactdisc
)


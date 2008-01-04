#include <QtCore>
#include <QtTest>

class AudioCD_test: public QObject {

	Q_OBJECT

	public:
		AudioCD_test();

		bool all_tests();

		bool getCdDrive_test();

		bool getCd_test();

		bool getMediaSource_test();

		bool getCdPath_test();
}



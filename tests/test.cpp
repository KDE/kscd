#include "hwcontroler_test.h"
#include "audiocd_test.h"

int main ()
{
	//Initialisation
	HWControler_test hwct = new HWControler_test();
	AudioCD_test acdt = new AudioCD_test();

	//Tests
	if (hwct->all_tests() && acdt->all_tests()) {
		put_line ("Tests OK");
	}
	else {
		put_line ("Errors on Tests");
	}
}
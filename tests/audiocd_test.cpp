AudioCD_test :: AudioCD_test()
{

}

bool AudioCD_test :: all_tests()
{
	return (getCdDrive_test() && getCd_test() && getMediaSource_test() && getCdPath_test());
}

bool AudioCD_test :: getCdDrive_test()
{
	return true;
}

bool AudioCD_test :: getCd_test()
{
	return true;
}

bool AudioCD_test :: getMediaSource_test()
{
	return true;
}

bool AudioCD_test :: getCdPath_test()
{
	return true;
}
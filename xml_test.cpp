/// XML Test

#include "xml.hpp"
#include <cmath>


int main(int argc, char** argv )
{
	std::ofstream test_fos("xml_output_test.xml");

	XML::writer	wr(test_fos);

	wr.beg("doc",1)
		.set_param<const char*>("name","test",XML::writer::INLINE);
		wr.beg("d",3UL)
			.set_param<float>("norm",	1.0f/sqrtf(2.0f))
			.set_param<float>("X",		1.0f)
			.set_param<float>("Y",		1.0f);
			wr.beg("p")() 
				<< "Just some words" << std::endl;
			wr.end("p");
		wr.end("d");

	test_fos.close();




	std::ifstream 	test_fis("xml_output_test.xml");
	std::string 	sval;
	float 			norm;
	float 			X;
	float 			Y;

	XML::reader	rd(test_fis);

	rd.next("doc")
		.get_param<std::string>("name",	sval);

	std::cout << sval << std::endl;

	rd.next("d")
		.get_param<float>("norm",	norm)
		.get_param<float>("X",		X)
		.get_param<float>("Y",		Y);

	std::cout 	<< "norm"	<< '\t' << norm	<< '\t'
				<< "X"		<< '\t' << X	<< '\t'
				<< "Y"		<< '\t' << Y	<< '\t' 
				<< std::endl;

	rd.into("p").get_content(std::cout);

	return 0;
}
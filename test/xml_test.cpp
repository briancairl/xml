/// XML Test
#include <iostream>
#include <cmath>
#include <string>

#include <xml/xml.h>

int main(int argc, char** argv )
{
	std::string fname = "xml_output_test.xml";

	/// Writer Test
	std::cout << "Creating an XML document" << std::endl;
	std::ofstream test_fos(fname.c_str());

	XML::writer	wr(test_fos);

	wr.beg("doc", 1UL)
		.set_param<std::string>("name", "test", XML::writer::INLINE);
		wr.beg("d", 4UL)
			.set_param<float>("norm",	1.0f/sqrtf(2.0f))
			.set_param<float>("X",		1.0f)
			.set_param<float>("Y",		1.0f)
			.set_param<float>("id",		1);
			wr.beg("p")() 
				<< "Just some words" << std::endl;
			wr.end("p");
		wr.end("d");

	test_fos.close();
	std::cout << "Done creating an XML document" << std::endl;


	/// Reader Test
	std::string sval;
	float 			norm;
	float 			X;
	float 			Y;
	int 				id;

	XML::reader	rd(fname.c_str());

	//<doc ... />
	rd.next("doc")
		.get_param<std::string>("name",sval);

	//<d>
	rd.next("d")
		.get_param<float>("norm",	norm)
		.get_param<float>("X",		X)
		.get_param<float>("Y",		Y)
		.get_param<int>("id",  id);

		//<p> ... </p>
		rd.into("p").get_content(std::cout);
	//</d>



	std::cout 
				<< sval 							<< std::endl
				<< "norm : " << norm	<< std::endl
				<< "X    : " << X	 		<< std::endl
				<< "Y    : " << Y	 		<< std::endl
				<< "ID   : " << id	 	<< std::endl;

	return 0;
}
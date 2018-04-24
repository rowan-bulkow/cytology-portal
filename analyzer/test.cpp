#include <iostream>
#include <boost/program_options.hpp>

using namespace std;
using namespace boost::program_options;

int main (int argc, const char * argv[])
{
    try
  {
    options_description desc{"Options"};
    desc.add_options()
      ("help,h", "Help screen")
      ("maxVariation", value<float>()->default_value(0.5f), "MaxVariation")
      ("minDiversity", value<float>()->default_value(0.25f), "MinDiversity")
      ("kernelsize", value<int>()->default_value(2), "Kernelsize")
      ("maxdist", value<int>()->default_value(4), "Maxdist")
      ("threshold1", value<int>()->default_value(20), "threshold1")
      ("threshold2", value<int>()->default_value(40), "threshold2")
      ("maxGmmIterations", value<int>()->default_value(10), "maxGmmIterations")
      ("delta", value<int>()->default_value(0), "delta")
      ("minArea", value<int>()->default_value(15), "minArea")
      ("maxArea", value<int>()->default_value(100), "maxArea")
      ("photo", value<std::string>()->default_value("cyto.tif"), "photo");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    cout << "here are all of my testing values"<< endl;
    cout << "maxVariation " << vm["maxVariation"].as<float>() << "   " << "minDiversity " << vm["minDiversity"].as<float>() << endl;
    cout << "kernelsize " << vm["kernelsize"].as<int>() <<  "     " << "maxdist " << vm["maxdist"].as<int>() << endl;
    cout << "threshold1 " << vm["threshold1"].as<int>() <<  "     " << "threshold2 " << vm["threshold2"].as<int>() << endl;
    cout << "maxGmmIterations " << vm["maxGmmIterations"].as<int>() <<  "     " << "delta " << vm["delta"].as<int>() << endl;
    cout << "minArea " << vm["minArea"].as<int>() <<  "     " << "maxArea " << vm["maxArea"].as<int>() << endl;
    cout << "photo " << vm["photo"].as<std::string>() << endl;

  


  }
  catch (const error &ex)
  {
    std::cerr << ex.what() << '\n';
  }
}
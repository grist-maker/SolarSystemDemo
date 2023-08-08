#include "pch.h"
#include "ModelProcessor.h"

using namespace std;
using namespace std::filesystem;
using namespace std::string_literals;
using namespace ModelPipeline;
using namespace Library;

int main(int argc, char* argv[])
{
	argc;
	argv;
/*#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		if (argc < 2)
		{
			throw exception("Usage: ModelPipeline.exe inputfilename");
		}

		path inputFile(argv[1]);
		current_path(inputFile.parent_path().c_str());

		cout << "Reading: "s << inputFile.filename() << endl;
		Model model = ModelProcessor::LoadModel(inputFile.filename().string(), true);

		string outputFilename = inputFile.stem().string() + ".model"s;
		if (!model.HasMeshes())
		{
			throw exception("Model has no meshes.");
		}
		
		cout << "Writing: "s << outputFilename << endl;
		model.Save(outputFilename);
		cout << "Finished."s << endl;
	}
	catch (exception ex)
	{
		cout << ex.what() << endl;
	}
	*/
	return 0;
}
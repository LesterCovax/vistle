/* wrapper for COVER to distribute environment variables to slaves before
 * initialization of static OpenSceneGraph objects happens */

#include <mpi.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <map>

#include <util/findself.h>
#include <util/sysdep.h>

#ifndef RENAME_MAIN
#error "RENAME_MAIN not defined"
#endif

#define xstr(s) #s
#define str(s) xstr(s)

int main(int argc, char *argv[]) {
#if defined(WIN32)
	const char libcover[] = "mpicover.dll";
#elif defined(__APPLE__)
	const char libcover[] = "libmpicover.dylib";
#else
	const char libcover[] = "libmpicover.so";
#endif
	const char mainname[] = str(RENAME_MAIN);

	std::string bindir = vistle::getbindir(argc, argv);

   int provided = MPI_THREAD_SINGLE;
   MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
   if (provided == MPI_THREAD_SINGLE) {
      std::cerr << "no thread support in MPI" << std::endl;
      exit(1);
   }

	int rank = -1;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	std::map<std::string, std::string> env;
	std::map<std::string, bool> envToSet;
	if (rank == 0) {
		std::vector<std::string> envvars;
		// system
		envvars.push_back("PATH");
		envvars.push_back("LD_LIBRARY_PATH");
		envvars.push_back("DYLD_LIBRARY_PATH");
		envvars.push_back("DYLD_FRAMEWORK_PATH");
		envvars.push_back("LANG");
		// covise config
		envvars.push_back("COCONFIG");
		envvars.push_back("COCONFIG_LOCAL");
		envvars.push_back("COCONFIG_DEBUG");
		envvars.push_back("COCONFIG_DIR");
		envvars.push_back("COCONFIG_SCHEMA");
		envvars.push_back("COVISE_CONFIG");
		// cover
		envvars.push_back("COVER_PLUGINS");
		envvars.push_back("COVER_TABLETPC");
		envvars.push_back("COVISE_SG_DEBUG");
		//envvars.push_back("COVISE_HOST");
		envvars.push_back("COVISEDIR");
		envvars.push_back("COVISE_PATH");
		envvars.push_back("ARCHSUFFIX");
		// OpenSceneGraph
		envvars.push_back("OSGFILEPATH");
		envvars.push_back("OSG_FILE_PATH");
		envvars.push_back("OSG_NOTIFY_LEVEL");
		envvars.push_back("OSG_LIBRARY_PATH");
		envvars.push_back("OSG_LD_LIBRARY_PATH");
		for (auto v : envvars) {

			const char *val = getenv(v.c_str());
			if (val)
				env[v] = val;
		}

		std::string covisedir = env["COVISEDIR"];
		std::string archsuffix = env["ARCHSUFFIX"];

		if (!covisedir.empty()) {
            std::string print_covise_env = covisedir + "/bin/print_covise_env";
#ifdef _WIN32
            print_covise_env += ".bat";
#endif
			if (FILE *fp = popen(print_covise_env.c_str(), "r")) {
				std::vector<char> buf(10000);
				while (fgets(buf.data(), buf.size(), fp)) {
					auto sep = std::find(buf.begin(), buf.end(), '=');
					if (sep != buf.end()) {
						std::string name = std::string(buf.begin(), sep);
						++sep;
						auto end = std::find(sep, buf.end(), '\n');
						std::string val = std::string(sep, end);
						//std::cerr << name << "=" << val << std::endl;
						env[name] = val;
					}
					//ld_library_path = buf.data();
					//std::cerr << "read ld_lib: " << ld_library_path << std::endl;
				}
				pclose(fp);
			}
		}
	}

    //std::string vistleplugin = bindir + "/../../../" + env["ARCHSUFFIX"] + "/lib/OpenCOVER/plugins/";
    std::string vistleplugin;
#ifdef __APPLE__
	vistleplugin += "libVistle.so";
#elif WIN32
	vistleplugin += "Vistle.dll";
#else
	vistleplugin += "libVistle.so";
#endif
#ifdef _WIN32
	std::cerr << "Vistle plugin: " << vistleplugin << std::endl;
    vistleplugin = "VistlePlugin";
#endif
	env["VISTLE_PLUGIN"] = vistleplugin;

	std::string ldpath, dyldpath, dyldfwpath, covisepath;

	int numvars = env.size();
	MPI_Bcast(&numvars, 1, MPI_INT, 0, MPI_COMM_WORLD);
	auto it = env.begin();
	for (int i = 0; i < numvars; ++i) {
		std::string name;
		std::string value;
		if (rank == 0) {
			name = it->first;
			value = it->second;
		}

		auto sync_string = [rank](std::string &s) {
			std::vector<char> buf;
			int len = -1;
			if (rank == 0)
				len = s.length() + 1;
			MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
			buf.resize(len);
			if (rank == 0)
				strcpy(buf.data(), s.c_str());
			MPI_Bcast(buf.data(), buf.size(), MPI_BYTE, 0, MPI_COMM_WORLD);
			s = buf.data();
		};
		sync_string(name);
		sync_string(value);

#if 0
		if (name == "COVISE_PATH") {
			// adapt in order to find VistlePlugin
			covisepath = bindir + "/../..";
			if (!value.empty()) {
				covisepath += ":";
				covisepath += value;
			}
			value = covisepath;
		}
#endif

		setenv(name.c_str(), value.c_str(), 1 /* overwrite */);

		if (rank == 0)
			++it;
		else
			env[name] = value;

		//std::cerr << name << " -> " << value << std::endl;
	}

	typedef int(*main_t)(int, char *[]);
	main_t realmain = NULL;
	int ret = 0;
#if 0
	std::string abslib = bindir + "/../../lib/" + libcover;
#else
	std::string coviselibdir = env["COVISEDIR"] + "/" + env["ARCHSUFFIX"] + "/lib/";
	std::string abslib = coviselibdir + libcover;
#endif
#ifdef WIN32
	void *handle = LoadLibraryA(abslib.c_str());
#else
	void *handle = dlopen(abslib.c_str(), RTLD_LAZY);
#endif

	if (!handle) {
#ifdef WIN32
		std::cerr << "failed to dlopen " << abslib << std::endl;
#else
		std::cerr << "failed to dlopen " << abslib << ": " << dlerror() << std::endl;
#endif
		ret = 1;
		goto finish;
	}

#ifdef WIN32
	realmain = (main_t)GetProcAddress((HINSTANCE)handle, mainname);;
#else
	realmain = (main_t)dlsym(handle, mainname);
#endif
	if (!realmain) {
		std::cerr << "could not find " << mainname << " in " << libcover << std::endl;
		ret = 1;
		goto finish;
	}

	ret = realmain(argc, argv);

finish:
	if (handle)
	{

#ifdef _WIN32
		FreeLibrary((HINSTANCE)handle);
#else
		dlclose(handle);
#endif
	}
   MPI_Finalize();

   return ret;
}

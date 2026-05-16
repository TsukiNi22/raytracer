/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file RaytracerInit.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#define _Write
#include "utils/utils.hpp"
#include "raytracer/Raytracer.hpp"
#include <libconfig.h++>
#include <unordered_set>
#include <filesystem>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <string>
#include <regex>

static void isDirectory(const std::string& path)
{
    if (!std::filesystem::is_directory(path))
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::InvalidDirectory, path);
}

static void isFile(const std::string& path, const std::string& extensionWanted = "")
{
    if (!std::filesystem::is_regular_file(path))
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::InvalidFile, path);

    // Check extension
    if (extensionWanted.empty()) return;
    if (std::filesystem::path(path).extension() != extensionWanted) {
        utils::exception::CustomException e(utils::exception::Warning, utils::exception::Code::InvalidFileExtension, std::string("Excepted extension: ") + extensionWanted + ", got: " + std::filesystem::path(path).extension().string());
        std::cout << e.formated() << std::endl;
    }
}

static nodiscard bool checkWritablePath(const std::string& rawPath)
{
    std::filesystem::path path(rawPath);

    // Explicit directory
    bool isDirectory = rawPath.back() == '/' || rawPath.back() == '\\';
    try {
        // Check if the directory exist
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
            isDirectory = true;

        // On directory case
        if (isDirectory) {
            std::filesystem::create_directories(path);
            std::filesystem::path testFile = path / ".TO_DELETE-permission_check_auto_generated_file";
            std::ofstream file(testFile.string());
            if (!file) return false;
            file.close();
            std::filesystem::remove(testFile);
            return true;
        }

        // File case
        std::filesystem::path parent = path.parent_path();
        if (!parent.empty()) std::filesystem::create_directories(parent);
        std::ofstream file(path.string(), std::ios::app);
        if (!file) return false;
        return true;

    } catch (...) { // Any error same as missing permission
        return false;
    }
}

void raytracer::Raytracer::load(int argc, char *argv[])
{
    const char* envVar = nullptr;
    int i = 1;

    // Check for a minimum of arguments
    if (argc < 2) unlikely {
        throw utils::exception::ErrorException(utils::exception::Code::ArgumentsNumber);
    }

    // Handle first argument <scene_cfg_path> || <ppm_file_path>
    if (std::filesystem::path(argv[i]).extension() != ".cfg"
        && std::filesystem::path(argv[i]).extension() != ".ppm") {
        if (argc > 2) { // cfg
            isFile(argv[i]);
            utils::exception::CustomException e(utils::exception::Warning, utils::exception::Code::InvalidFileExtension, "The first file given was interpreted has a '.cfg'");
            std::cout << e.formated() << std::endl;
            this->_settings.viewer = false;
            this->_settings.cfg_path = argv[i];
        } else { // ppm
            isFile(argv[i]);
            utils::exception::CustomException e(utils::exception::Warning, utils::exception::Code::InvalidFileExtension, "The first file given was interpreted has a '.ppm'");
            std::cout << e.formated() << std::endl;
            this->_settings.viewer = true;
            this->_settings.ppm_path = argv[i];
            return;
        }
    } else if (std::filesystem::path(argv[i]).extension() == ".cfg") {
        isFile(argv[i], ".cfg");
        this->_settings.viewer = false;
        this->_settings.cfg_path = argv[i];
    } else {
        if (std::filesystem::path(argv[i]).extension() != ".ppm") {
            utils::exception::CustomException e(utils::exception::Warning, utils::exception::Code::InvalidFileExtension, "The first file given was interpreted has a '.ppm'");
            std::cout << e.formated() << std::endl;
        } else {
            isFile(argv[i], ".ppm");
        }
        this->_settings.viewer = true;
        this->_settings.ppm_path = argv[i];
        return;
    }
    ++i;

    // Is in gui mode?
    if (argc > i && std::string(argv[i]) == "-gui") {
        this->_settings.gui = true;
        ++i;
    }

    // Global argument handling
    std::string arg;
    for (; i < argc; ++i) {
        arg = std::string(argv[i]);

        // -d, --debug
        if (arg == "-d" || arg == "--debug") {
            // Check the option argument
            if (this->_settings.debug) {
                utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::OptionOverride, arg);
                std::cout << e.formated() << std::endl;
            }

            this->_settings.debug = true;
        }

        // -a, --advencement
        else if (arg == "-a" || arg == "--advencement") {
            // Check the option argument
            if (this->_settings.adv) {
                utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::OptionOverride, arg);
                std::cout << e.formated() << std::endl;
            }

            this->_settings.adv = true;
        }

        // -g, --newton
        else if (arg == "-g" || arg == "--newton") {
            // Check the option argument
            if (this->_settings.newton) {
                utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::OptionOverride, arg);
                std::cout << e.formated() << std::endl;
            }

            // Set the newton mode
            this->_settings.newton = true;
        }

        // -n, --nproc
        else if (arg == "-n" || arg == "--nproc") {
            if (i + 1 >= argc)
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::MissingOptionArgument, "-n requires <nproc>");

            // Check the option argument
            if (this->_settings.nproc_set) {
                utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::OptionOverride, arg);
                std::cout << e.formated() << std::endl;
            }
            try {
                std::string str = argv[++i];
                if (str.empty())
                    throw std::invalid_argument("empty");
                if (!std::all_of(str.begin(), str.end(), ::isdigit))
                    throw std::invalid_argument("not numeric");
                unsigned long long tmp = std::stoull(str);
                if (tmp > std::numeric_limits<std::size_t>::max())
                    throw std::out_of_range("overflow");
                this->_settings.nproc = static_cast<std::size_t>(tmp);
            } catch (const std::exception& e) {
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::MissingOptionArgument, std::string("-n Invalid number of processus: ") + e.what());
            }
            if (this->_settings.nproc != 0 && this->_settings.nproc < 2)
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::MissingOptionArgument, "-n Invalid number of processus, need to be superior to 1 or equal to 0 (auto)");

            // Set the nproc value
            this->_settings.nproc_set = true;
        }

        // -c, --camera <camera_file_path>
        else if (arg == "-c" || arg == "--camera") {
            if (i + 1 >= argc)
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::MissingOptionArgument, "-c requires <camera_file_path>");

            // Check the option argument
            if (this->_settings.camera_set) {
                utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::OptionOverride, arg);
                std::cout << e.formated() << std::endl;
            }
            isFile(argv[++i], ".so");

            // Set the path to the camera used
            this->_settings.camera_set = true;
            this->_settings.camera_path = argv[i];
        }

        // -p, --plugins <plugins_directory_path>
        else if (arg == "-p" || arg == "--plugins") {
            if (i + 1 >= argc)
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::MissingOptionArgument, "-p requires <plugins_directory_path>");

            // Check the option argument
            if (this->_settings.plugins_set) {
                utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::OptionOverride, arg);
                std::cout << e.formated() << std::endl;
            }
            isDirectory(argv[++i]);

            // Set the plugins search path
            this->_settings.plugins_set = true;
            this->_settings.plugins_path = argv[i];
        }

        // -o, --obj <obj_directory_path>
        else if (arg == "-o" || arg == "--obj") {
            if (i + 1 >= argc)
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::MissingOptionArgument, "-o requires <obj_directory_path>");

            // Check the option argument
            if (this->_settings.obj_set) {
                utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::OptionOverride, arg);
                std::cout << e.formated() << std::endl;
            }
            isDirectory(argv[++i]);

            // Set the plugins search path
            this->_settings.obj_set = true;
            this->_settings.obj_path = argv[i];
        }

        // -s, --save (<ppm_directory_path>|<ppm_file_path>)
        else if (arg == "-s" || arg == "--save") {
            if (i + 1 >= argc)
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::MissingOptionArgument, "-s requires <ppm_directory_path> or <ppm_file_path>");

            // Check the option argument
            if (this->_settings.rendered_set) {
                utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::OptionOverride, arg);
                std::cout << e.formated() << std::endl;
            }
            if (!checkWritablePath(argv[++i]))
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::MissingOptionArgument, "-s Invalid path given, can't output a file here");

            // Ignored case
            if (this->_settings.gui) {
                utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::IgnoredArgument, arg);
                std::cout << e.formated() << std::endl;
            }

            // Set the ppm save path
            this->_settings.rendered_set = true;
            this->_settings.rendered_path = argv[i];
        }

        // -r, --resolution "wxh"
        else if (arg == "-r" || arg == "--resolution") {
            if (i + 1 >= argc)
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::MissingOptionArgument, "-r requires \"wxh\"");

            // Check the option argument
            if (this->_settings.resolution_set) {
                utils::exception::CustomException e(utils::exception::Type::Warning, utils::exception::Code::OptionOverride, arg);
                std::cout << e.formated() << std::endl;
            }
            std::string res = argv[++i];
            auto pos = res.find('x');
            if (pos == std::string::npos)
                throw utils::exception::CustomException(utils::exception::Type::Warning, utils::exception::Code::InvalidArgument, std::string("Invalid resolution format: ") + res);

            // Try to set the resolution
            try {
                this->_settings.resolution.x = std::stoul(res.substr(0, pos));
                this->_settings.resolution.y = std::stoul(res.substr(pos + 1));
            } catch (const std::exception& e) {
                throw utils::exception::CustomException(utils::exception::Warning, utils::exception::Code::InvalidArgument, std::string("Invalid resolution format: ") + res);
            }
            this->_settings.resolution_set = true;
        }

        // Ignored argument
        else {
            utils::exception::CustomException e(utils::exception::Warning, utils::exception::Code::InvalidArgument, arg);
            std::cout << e.formated() << std::endl;
        }
    }

    // Try env var getting if the arguments din't already set it
    if (!this->_settings.nproc_set) {
        if ((envVar = std::getenv("RAYTRACER_NPROC"))) {
            try {
                std::string str = argv[i];
                if (str.empty())
                    throw std::invalid_argument("empty");
                if (!std::all_of(str.begin(), str.end(), ::isdigit))
                    throw std::invalid_argument("not numeric");
                unsigned long long tmp = std::stoull(str);
                if (tmp > std::numeric_limits<std::size_t>::max())
                    throw std::out_of_range("overflow");
                this->_settings.nproc = static_cast<std::size_t>(tmp);
            } catch (const std::exception& e) {
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::MissingOptionArgument, std::string("-n Invalid number of processus: ") + e.what());
            }
        } /*else {
            utils::exception::CustomException e(utils::exception::Warning, utils::exception::Code::MissingEnvVar, "Wasn't able to find the environement variable: RAYTRACER_PLUGINS_PATH");
            std::cout << e.formated() << std::endl;
        }*/
    }
    if (!this->_settings.plugins_set) {
        if ((envVar = std::getenv("RAYTRACER_PLUGINS_PATH"))) {
            this->_settings.plugins_path = envVar;
        } /*else {
            utils::exception::CustomException e(utils::exception::Warning, utils::exception::Code::MissingEnvVar, "Wasn't able to find the environement variable: RAYTRACER_PLUGINS_PATH");
            std::cout << e.formated() << std::endl;
        }*/
    }
    if (!this->_settings.rendered_set) {
        if ((envVar = std::getenv("RAYTRACER_RENDERED_PATH"))) {
            this->_settings.rendered_path = envVar;
        } /*else {
            utils::exception::CustomException e(utils::exception::Warning, utils::exception::Code::MissingEnvVar, "Wasn't able to find the environement variable: RAYTRACER_RENDERED_PATH");
            std::cout << e.formated() << std::endl;
        }*/
    }
}

static nodiscard std::vector<std::filesystem::path> getPlugins(const std::filesystem::path& root)
{
    std::vector<std::filesystem::path> plugins;

    // Check the root permission & type
    if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root))
        return plugins;

    // Get recursively the file
    for (const auto& entry: std::filesystem::recursive_directory_iterator(root)) {
        if (entry.is_regular_file() && entry.path().extension() == ".so") likely {
            plugins.push_back(entry.path());
        }
    }

    return plugins;
}

void raytracer::Raytracer::init(void)
{
    std::vector<std::filesystem::path> plugins = getPlugins(this->_settings.plugins_path);
    std::shared_ptr<raytracer::DynamicLibrary> lib;
    std::size_t type = 0;
    std::string name;
    bool selectedCamera = false;

    // For each of them try to load it and dispatch it
    if (plugins.size() == 0) unlikely {
        throw utils::exception::ErrorException(utils::exception::Code::NoPlugins);
    }
    std::cout << utils::write::strong() << plugins.size() << utils::write::reset() << ": plugins where found" << std::endl;

    // Load each
    for (const std::filesystem::path& path: plugins) {
        // Try to load the lib
        lib = std::make_shared<raytracer::DynamicLibrary>(path);

        selectedCamera |= (!this->_settings.camera_set || (this->_settings.camera_set && std::filesystem::equivalent(path, this->_settings.camera_path)));

        // Continue only if the lib was succefully loaded
        if (lib->isloaded()) likely {
            std::cout << utils::write::strong() << path << utils::write::reset() << ": plugin succefully " << utils::write::color_rgb(0, 255, 0) << "loaded" << utils::write::reset() << std::endl;
            type = lib->getType();
            name = lib->getName();

            // Selected camera
            if (type == CAMERA && selectedCamera && !this->_camera) {
                try {
                    raytracer::ICamera* (*factory)() = lib->loadFunction<raytracer::ICamera* (*)()>("factory");
                    this->_camera = factory();
                    std::cout << utils::write::strong() << path << utils::write::reset() << ": camera selected" << std::endl;
                } catch (const utils::exception::IException&) {
                    std::cout << utils::write::strong() << path << utils::write::reset() << ": the camera wasn't " << utils::write::color_rgb(255, 0, 0) << "created" << utils::write::reset() << std::endl;
                    throw;
                }
            }

            // Plugins type check (just stored)
            if (type == CAMERA || type == LIGHT || type == OBJECT || type == MATERIAL) {
                std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                this->_libs[name + std::to_string(type)] = lib;
            } else unlikely {
                std::cout << utils::write::color_rgb(0, 255, 0) << utils::write::strong() << path << utils::write::reset() << ": plugins has a different type than those accepted" << std::endl;
                continue;
            }
        } else unlikely {
            std::cout << utils::write::strong() << path << utils::write::reset() << ": plugin not " << utils::write::color_rgb(255, 0, 0) << "loaded" << utils::write::reset() << std::endl;
        }
    }
    
    // Check camera loading
    if (!this->_camera) unlikely {
        throw utils::exception::ErrorException(utils::exception::Code::NoLoadedCamera);
    }

    // Init the scene with the config file
    this->scene();

    // Init rays
    this->advReset(1 + this->_lights.size());
    if (this->_settings.resolution_set)
        this->_camera->setResolution(this->_settings.resolution);
    this->_camera->init();
    this->adv(true);
    for (raytracer::ILight* light: this->_lights) {
        light->init();
        this->adv(true);
    }
    this->advFull();
    this->advEnd();

    // Init internal optimisation for SDF
    for (raytracer::IObject* object: this->_objects) {
        for (const raytracer::Chunk& chunk: object->getObjectDescriptor().chunks)
            this->_objectsChunks[chunk].push_back(object);
    }
}

static std::string getFileContent(const std::filesystem::path& path)
{
    std::ifstream file(path);
    std::stringstream buffer;

    // Check the file opening
    if (!file)
        throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "File IO error");

    // Red the file content
    buffer << file.rdbuf();

    return buffer.str();
}

void raytracer::Raytracer::scene(void)
{
    std::stringstream buffer;
    std::string content;
    libconfig::Config cfg;

    // Get the config file content
    content = getFileContent(this->_settings.cfg_path);

    // Pre parse the config file for custom instruction
    this->parseSceneFile(this->_settings.cfg_path, content);

    // Try to load the config file
    try {
        cfg.readString(content);
    } catch (const libconfig::ParseException& e) {
        throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, std::string("Parse error at ") + this->_settings.cfg_path + ":" + std::to_string(e.getLine()) + " - " + e.getError());
    }

    // For each node in the config
    const libconfig::Setting& root = cfg.getRoot();
    bool camera = false;
    std::string type;
    for (int i = 0; i < root.getLength(); ++i) {
        // Get the node
        const libconfig::Setting& node = root[i];

        // Get the type of the node
        node.lookupValue("type", type);
        std::transform(type.begin(), type.end(), type.begin(), ::tolower);

        // Dispatch the node constructor
        if (type == "sky") {
            if (this->_sky.isEnabled())
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "Can't set multiple sky on the scene");
            this->_sky.enable();

            // Set the sky color, by default: DEFAULT_COLOR
            if (node.exists("color")) {
                const libconfig::Setting& color = node["color"];
                this->_sky.setColor({
                    (int)color[0],
                    (int)color[1],
                    (int)color[2]
                });
            }
        } else if (type == "camera") {
            if (camera)
                throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "Can't set multiple camera on the scene");
            this->_camera->parse(node);
            camera = true;
        } else if (type == "light") {
            this->parseLight(node);
        } else if (type == "object") {
            this->parseObject(node);
        } else {
            throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "Unknow node type: " + type);
        }
    }
    if (!camera)
        throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "The camera wasn't set in the scene");
}

void raytracer::Raytracer::parseSceneFile(const std::string& path, std::string& content) const
{
    static std::unordered_set<std::string> visited; // Used for infinite include loop detection
    std::filesystem::path basePath = std::filesystem::absolute(path).parent_path();

    // Regex that match the different possible include
    std::regex pattern(R"(\$(import|include|paste)\s*\{([^}]*)\})");
    std::smatch match;

    // For each include match
    while (std::regex_search(content, match, pattern)) {
        // Get the match content
        std::string fullMatch = match[0];
        std::string filesList = match[2];

        // Setup the var used for the replacement
        std::stringstream ss(filesList);
        std::string replacement;
        std::string file;

        // For each file given to the include
        while (std::getline(ss, file, ',')) {
            // Clean the file name, apply trim & remove the '"'
            file.erase(remove_if(file.begin(), file.end(), ::isspace), file.end());
            if (file.front() == '"') file.erase(0, 1);
            if (file.back() == '"') file.pop_back();

            // Set the path to the file to include
            std::filesystem::path fullPath = basePath / file;
            fullPath = std::filesystem::absolute(fullPath);

            // Counter infinite include loop
            if (visited.contains(fullPath.string())) continue;
            visited.insert(fullPath.string());

            // Get the content
            std::string subContent = getFileContent(fullPath);
            this->parseSceneFile(fullPath.string(), subContent); // Check for include of the sub file(s)
            replacement += "\n" + subContent + "\n"; // Add \n to properly separate the different content
        }

        // Replace the include with the file(s) content
        content.replace(match.position(0), match.length(0), replacement);
    }
}

raytracer::IMaterial* raytracer::Raytracer::parseMaterial(const libconfig::Setting& node) const
{
    // Get the light name
    std::string name;
    node.lookupValue("name", name);
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    // Create the material using factory & call it's parser
    raytracer::IMaterial* material = this->factory<raytracer::IMaterial>(name + std::to_string(MATERIAL));
    material->parse(node);
    this->_materials.push_back(material);
    return material;
}

void raytracer::Raytracer::parseLight(const libconfig::Setting& node)
{
    // Get the light name
    std::string name;
    node.lookupValue("name", name);
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    // Create the light using factory & call it's parser
    raytracer::ILight* light = this->factory<raytracer::ILight>(name + std::to_string(LIGHT));
    light->parse(node);
    this->_lights.push_back(light);
}

void raytracer::Raytracer::parseObject(const libconfig::Setting& node)
{
    // Get the object name
    std::string name;
    node.lookupValue("name", name);
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    // Create the object using factory & call it's parser
    raytracer::IObject* object = this->factory<raytracer::IObject>(name + std::to_string(OBJECT));
    object->parse(node);
    raytracer::ObjectDescriptor& descriptor = object->getObjectDescriptor();

    // Init the material (mandatory on object)
    if (!node.exists("material"))
        throw utils::exception::CustomException(utils::exception::Error, utils::exception::Code::Parser, "The material field isn't defined for the object");
    descriptor.material = this->parseMaterial(node["material"]);

    // Init the obj if set
    if (!descriptor.obj.empty())
        object->loadObj(this->ObjPath(descriptor.obj));
    else if (node.lookupValue("obj", descriptor.obj))
        object->loadObj(this->ObjPath(descriptor.obj));

    this->_objects.push_back(object);
}

raytracer::Raytracer::~Raytracer()
{
    if (this->_camera)
        delete this->_camera;
    for (raytracer::ILight* light: this->_lights)
        delete light;
    for (raytracer::IObject* object: this->_objects)
        delete object;
    for (raytracer::IMaterial* material: this->_materials)
        delete material;
}

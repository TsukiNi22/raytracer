/**************************************************************\
Edition:
##  @date 15/05/2026 by @author Tsukini

File Name:
##  @file main.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Attribute
#define _Exception
#define _Write
#include "utils/utils.hpp"
#include "raytracer/Raytracer.hpp"
#include <iostream>

static cold void printHelp()
{
    std::cout << utils::write::format("<strong>PROJECT<>") << std::endl;
    std::cout << utils::write::color(utils::write::Color::Cyan) << "\tThis project aim to make a 'full' raytracer the " << utils::write::color(utils::write::Color::Yellow) << "'raytracer'" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Cyan) << "\tYou can make your own plugins and use them with our documentation" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Cyan) << "\tIt's should be able to run on the raytracer if the documentation is respected!!!" << std::endl;
    std::cout << utils::write::reset() << std::endl;

    std::cout << utils::write::format("<strong>USAGE<>") << std::endl;
    std::cout << utils::write::color(utils::write::Color::Magenta);
    std::cout << "\t./raytracer <scene_cfg_path> [-a] [-n <nproc>] [-c <camera_file_path>] [-p <plugins_directory_path>] [-o <obj_directory_path>] [-s (<ppm_directory_path>|<ppm_file_path>)] [-r \"wxh\"]" << std::endl;
    std::cout << "\t./raytracer <scene_cfg_path> -gui [-n <nproc>] [-c <camera_file_path>] [-p <plugins_directory_path>] [-o <obj_directory_path>] [-r \"wxh\"]" << std::endl;
    std::cout << "\t./raytracer <ppm_file_path>" << std::endl;
    std::cout << "\t./raytracer -h" << std::endl;
    std::cout << utils::write::reset() << std::endl;
    
    std::cout << utils::write::format("<strong>INFORMATION<>") << std::endl;
    std::cout << utils::write::color(utils::write::Color::Magenta) << "\t./raytracer <ppm_file_path>" << utils::write::reset() << "\tSwitch to ppm viewer mode and display the given ppm file" << std::endl;
    std::cout << utils::write::reset() << std::endl;

    std::cout << utils::write::format("<strong>OPTIONS<>") << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t-h, --help" << utils::write::reset() << std::endl;
    std::cout << "\t\tWrite the informations of the executable" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t-gui" << utils::write::reset() << std::endl;
    std::cout << "\t\tActivate the render at runtime" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t-a, --advencement" << utils::write::reset() << std::endl;
    std::cout << "\t\tEnable the advencement display of the actual frame rendering " << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t-g, --newton " << utils::write::reset() << "<" << utils::write::color(utils::write::Color::Red) << "delta" << utils::write::reset() << ">" << std::endl;
    std::cout << "\t\tSet the number of processus (0 = auto), can also be set by the env var RAYTRACER_NPROC (default: " << utils::write::color(utils::write::Color::Red) << "0" << utils::write::reset() << ")" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t-n, --nproc " << utils::write::reset() << "<" << utils::write::color(utils::write::Color::Red) << "np_proc" << utils::write::reset() << ">" << std::endl;
    std::cout << "\t\tActivate the newton mode, apply aproximative gravity on rays" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t-c, --camera " << utils::write::reset() << "<" << utils::write::color(utils::write::Color::Red) << "camera_file_path" << utils::write::reset() << ">" << std::endl;
    std::cout << "\t\tForce the camera plugin used (default: first found)" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t-p, --plugins " << utils::write::reset() << "<" << utils::write::color(utils::write::Color::Red) << "plugins_directory_path" << utils::write::reset() << ">" << std::endl;
    std::cout << "\t\tSet the plugins path, can also be set by the env var RAYTRACER_PLUGINS_PATH (default: \"" << utils::write::color(utils::write::Color::Red) << "./plugins/" << utils::write::reset() << "\")" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t-o, --obj " << utils::write::reset() << "<" << utils::write::color(utils::write::Color::Red) << "obj_directory_path" << utils::write::reset() << ">" << std::endl;
    std::cout << "\t\tSet the obj path, can also be set by the env var RAYTRACER_OBJ_PATH (default: \"" << utils::write::color(utils::write::Color::Red) << "./obj/" << utils::write::reset() << "\")" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t-s, --save " << utils::write::reset() << "(<" << utils::write::color(utils::write::Color::Red) << "ppm_directory_path" << utils::write::reset() << ">|<" << utils::write::color(utils::write::Color::Red) << "ppm_file_path" << utils::write::reset() << ">)" << std::endl;
    std::cout << "\t\tSet the ppm save path (directory: name = auto), can also be set by the env var RAYTRACER_RENDERED_PATH (default: \"" << utils::write::color(utils::write::Color::Red) << "./rendered/" << utils::write::reset() << "\")" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t-r, --resolution " << utils::write::reset() << "\"" << utils::write::color(utils::write::Color::Red) << "w" << utils::write::reset() << "x" << utils::write::color(utils::write::Color::Red) << "h" << utils::write::reset() << "\"" << std::endl;
    std::cout << "\t\tForce the camera resolution used (default: set in the cfg)" << std::endl;
    std::cout << utils::write::reset() << std::endl;

    std::cout << utils::write::format("<strong>INPUT<> (only -gui)") << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\tESC" << utils::write::reset() << "\tLeave the rendering" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\tr" << utils::write::reset() << "\tReset camera position" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\tq" << utils::write::reset() << "\tTranslate camera left" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\td" << utils::write::reset() << "\tTranslate camera right" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\tz" << utils::write::reset() << "\tTranslate camera forward" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\ts" << utils::write::reset() << "\tTranslate camera backward" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\te" << utils::write::reset() << "\tTranslate camera up" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\ta" << utils::write::reset() << "\tTranslate camera down" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t←" << utils::write::reset() << "\tRotate camera left" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t→" << utils::write::reset() << "\tRotate camera right" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t↑" << utils::write::reset() << "\tRotate camera up" << std::endl;
    std::cout << utils::write::color(utils::write::Color::Green) << "\t↓" << utils::write::reset() << "\tRotate camera down" << std::endl;
    std::cout << utils::write::reset() << std::flush;
}

static cold void run(raytracer::Raytracer& raytracer, int argc, char *argv[])
{
    // Init the settings
    raytracer.load(argc, argv);
    if (!raytracer.isViewer()) raytracer.init();

    // Change the start depending on the settings
    if (raytracer.isGui() || raytracer.isViewer()) {
        raytracer.gui(); // Launch gui
    } else {
        raytracer.light(); // Update light rendering
        raytracer.render(); // Update camera rendering
        raytracer.saveRender(); // Save the updated rendering
    }
}

int main(int argc, char *argv[])
{
    // Init the raytracer (in the main to keep the longer the plugins charged)
    raytracer::Raytracer raytracer;

    // Basic flag help detection
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") unlikely {
            printHelp();
            return OK;
        }
    }

    try {
        run(raytracer, argc, argv);
    } catch (const utils::exception::IException& e) { // For our own custom error
        if (e.isNone()) return OK; // Intercept exception of type None such as exit
        std::cerr << e.formated() << std::endl;
        return KO;
    } catch (const std::exception& e) { // For error from plugins library that haven't compatible error
        std::cerr << e.what() << std::endl;
        return KO;
    }

    return OK;
}

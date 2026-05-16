/**************************************************************\
Edition:
##  @date 16/05/2026 by @author Tsukini

File Name:
##  @file Raytracer.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Vector
#define _Attribute
#include "utils/utils.hpp"
#include "raytracer/special/Utils.hpp"
#include "raytracer/Raytracer.hpp"
#include "raytracer/cameras/Viewer.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <atomic>
#include <vector>
#include <cmath>

/*
gui:
    light
    load gui
    loop:
        input handling
        render
        display
    close gui

viewer:
    load gui
    load ppm
    display
    loop:
        input handling
    close gui
*/
cold void raytracer::Raytracer::gui(void)
{
    // Init screen data from save (viewer mode)
    if (this->_settings.viewer)
        this->loadRender();
    else // Pre light computing
        this->light();

    // Open display
    raytracer::Resolution resolution = this->_camera->getResolution();
    sf::VideoMode mode(resolution.x, resolution.y);
    sf::RenderWindow window(mode, "Raytracer", sf::Style::Titlebar | sf::Style::Close);

    // Pre loop (viewer mode)
    if (this->_settings.viewer)
        this->display(window);

    // Update display
    loop(window);

    // Close display
    window.close();

    // End of the advencement display
    this->advEnd();
}

// Run in a separated thread
static void subLoop(const sf::RenderWindow& window, raytracer::Raytracer& raytracer)
{
    if (!raytracer.isGui()) return;
    while (window.isOpen()) raytracer.render();
}

void raytracer::Raytracer::loop(sf::RenderWindow& window)
{
    std::thread subThread(subLoop, std::ref(window), std::ref(*this));
    sf::Event event;

    // Loop while the window is open
    while (window.isOpen()) {

        // Listen event
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) // Window killed
                window.close();
            if (event.type == sf::Event::KeyPressed) { // Camera movement
                raytracer::CFrame cframe = this->_camera->getCFrame();
                switch (event.key.code) {
                    // Window kill
                    case sf::Keyboard::Escape:
                        window.close();
                        break;

                    // Reset
                    case sf::Keyboard::R:
                        cframe = this->_camera->getCFrameOrigin();
                        break;

                    // Translation
                    case sf::Keyboard::Q:
                        cframe.position.x += MOVE_SPEED;
                        break;

                    case sf::Keyboard::D:
                        cframe.position.x -= MOVE_SPEED;
                        break;

                    case sf::Keyboard::Z:
                        cframe.position.z += MOVE_SPEED;
                        break;

                    case sf::Keyboard::S:
                        cframe.position.z -= MOVE_SPEED;
                        break;

                    case sf::Keyboard::E:
                        cframe.position.y += MOVE_SPEED;
                        break;

                    case sf::Keyboard::A:
                        cframe.position.y -= MOVE_SPEED;
                        break;

                    // Rotation
                    case sf::Keyboard::Left:
                        cframe.orientation.y += ORIENTATION_SPEED;
                        break;

                    case sf::Keyboard::Right:
                        cframe.orientation.y -= ORIENTATION_SPEED;
                        break;

                    case sf::Keyboard::Up:
                        cframe.orientation.x += ORIENTATION_SPEED;
                        break;

                    case sf::Keyboard::Down:
                        cframe.orientation.x -= ORIENTATION_SPEED;
                        break;

                    case sf::Keyboard::W:
                        cframe.orientation.z += ORIENTATION_SPEED;
                        break;

                    case sf::Keyboard::X:
                        cframe.orientation.z -= ORIENTATION_SPEED;
                        break;

                    default: break;
                }
                cframe.look = raytracer::toLook(cframe.orientation);
                this->_camera->setCFrame(cframe, false);
            }

        }

        // Update display content (gui mode)
        if (this->_settings.gui)
            this->display(window);
    }

    // Wait for the thread
    if (subThread.joinable())
        subThread.join();
}

hot void raytracer::Raytracer::display(sf::RenderWindow& window)
{
    // Clear the window
    window.clear(sf::Color::Black);

    // Update screen pixels using rays color
    this->_camera->updateScreen();

    // Draw each pixel
    raytracer::Resolution resolution = this->_camera->getResolution();
    const std::vector<raytracer::Color>& pixels = this->_camera->getScreen();
    sf::VertexArray points(sf::Points, resolution.x * resolution.y);
    for (std::uint16_t y = 0; y < resolution.y; ++y) {
        for (std::uint16_t x = 0; x < resolution.x; ++x) {
            const raytracer::Color& color = pixels[y * resolution.x + x];
            points[y * resolution.x + x].position = sf::Vector2f(x, y);
            points[y * resolution.x + x].color = sf::Color(color.x, color.y, color.z);
        }
    }

    // Font loading
    static sf::Font font;
    static bool loaded = false;
    if (!loaded) {
        font.loadFromFile("/usr/share/fonts/liberation-mono/LiberationMono-Regular.ttf");
        loaded = true;
    }

    // Setup the string
    const raytracer::CFrame& cframe = this->_camera->getCFrame();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "Position: ";
    ss << "(" << cframe.position.x;
    ss << ", " << cframe.position.y;
    ss << ", " << cframe.position.z << ")\n";
    ss << "Orientation: ";
    ss << "(" << cframe.orientation.x;
    ss << ", " << cframe.orientation.y;
    ss << ", " << cframe.orientation.z << ")\n";
    ss << "Look: ";
    ss << "(" << cframe.look.x;
    ss << ", " << cframe.look.y;
    ss << ", " << cframe.look.z << ")";

    // Setup the text
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(12);
    text.setFillColor(sf::Color::White);
    text.setString(ss.str());
    text.setPosition(15.f, 10.f);

    // Setup the text background
    sf::FloatRect bounds = text.getLocalBounds();
    sf::RectangleShape background;
    background.setPosition(8.f, 8.f);
    background.setSize({
        bounds.width + 15.0f,
        bounds.height + 10.0f
    });
    background.setFillColor(
        sf::Color(20, 20, 20, 180)
    );

    // Draw the window content
    window.draw(points);
    if (this->_settings.debug) {
        window.draw(background);
        window.draw(text);
    }

    // Update the display
    window.display();
}

static hot void processLightChunk(raytracer::Raytracer& raytracer,
    std::vector<raytracer::LightRay*>& rays, std::size_t start, std::size_t end,
    const std::vector<raytracer::IObject*>& objects,
    const std::unordered_map<raytracer::Chunk, std::vector<raytracer::IObject*>, raytracer::ChunkHash>& objectsChunks,
    raytracer::ICamera* camera,
    std::size_t depth = 0)
{
    std::vector<raytracer::LightRay*> raysClones;
    raytracer::IObject* nearestObject = nullptr;
    raytracer::Type distanceUnit = 0.0;
    const raytracer::IMaterial* material = nullptr;
    const raytracer::Face* faceHit = nullptr;
    raytracer::Direction look;
    raytracer::Angle angle = 0.0;
    float t = 0.0f;

    // Check depth
    if (depth > RAY_MAX_DEPTH) return;

    for (std::size_t i = start; i < end; ++i) {
        raytracer::LightRay* ray = rays[i];
        distanceUnit = ray->getCFrame().look.length();
        while (ray->isAlive()) {
            // Kill those with no direction
            if (ray->getCFrame().look <= 1e-8 && ray->getCFrame().look >= -1e-8) {
                ray->kill();
                continue;
            }

            // Update the different hits
            ray->computeObjects(camera->getRenderDistance(), objects, objectsChunks);

            // 1 - Compute T
            t = 0.0f;
            faceHit = nullptr;
            nearestObject = nullptr;
            for (const auto &[object, actualT, face]: ray->getHits()) {
                //if (ray->getImmunity() == object) continue;
                if (!nearestObject || actualT < t) {
                    t = actualT;
                    faceHit = face;
                    nearestObject = object;
                }
            }
            if (!nearestObject) { // No valid T
                ray->kill();
                continue;
            }

            // 2 - Apply T (aproximative gravity curve, only in newton mode)
            ray->translate(ray->getCFrame().look * t);
            ray->addDistance(distanceUnit * t);

            // Kill conditions
            if (std::isnan(t) || (ray->getCFrame().position - camera->getCFrame().position).lengthSquared() >= camera->getRenderDistance() * camera->getRenderDistance()) { // Too far
                ray->kill();
                continue;
            } else if (ray->getDistance() >= camera->getRenderDistance() * RAY_DISTANCE_COEF) { // Live distance
                ray->kill();
                continue;
            } else if (ray->getIntensity() <= LIGHT_INTENSITY_LIMIT) { // Too low intensity
                ray->kill();
                continue;
            }

            // 3 - Check T
            if (ray->getImmunity() != nearestObject) { // Collision
                material = nearestObject->getObjectDescriptor().material;
                // Transparency & Refraction
                if (material->getTransparency() > 1e-8) {
                    raysClones.push_back(static_cast<raytracer::LightRay*>(ray->clone()));
                    raysClones.back()->setImmunity(nearestObject);
                    raysClones.back()->setIntensity(raysClones.back()->getIntensity() * material->getTransparency() * (1.0f - material->getReflection()));
                    raysClones.back()->setColor(raytracer::mergeColor(material->getColor(), raysClones.back()->getColor(), raysClones.back()->getIntensity()));
                }

                // Normal computing
                look = ray->getCFrame().look;
                nearestObject->reflectRay(ray, faceHit);
                angle = raytracer::radToDeg(std::atan2(look.dot(ray->getCFrame().look), look.length() * ray->getCFrame().look.length()));
                float localIntensityCoef = 1.0f - (angle / 180.0f); // 0° = 1.0f, 180° = 0.0f
                nearestObject->addLightData(ray->getCFrame().position, ray->getColor(), ray->getLuminescence() * localIntensityCoef * (1.0f - material->getReflection()));
                ray->setIntensity(ray->getIntensity() * material->getReflection());
                ray->setColor(raytracer::mergeColor(material->getColor(), ray->getColor(), ray->getIntensity()));

                // To counter collision with the same object on the next iteration
                ray->translate(ray->getCFrame().look * COLLISION_COUNTER_COEF);
                ray->setImmunity(nearestObject);
            } else { // Collision on the other side
                nearestObject->addLightData(ray->getCFrame().position, ray->getColor(), ray->getLuminescence());

                // To counter collision with the same object on the next iteration
                ray->translate(ray->getCFrame().look * COLLISION_COUNTER_COEF);
            }

            // Kill conditions
            if (std::isnan(t) || (ray->getCFrame().position - camera->getCFrame().position).lengthSquared() >= camera->getRenderDistance() * camera->getRenderDistance()) { // Too far
                ray->kill();
            } else if (ray->getDistance() >= camera->getRenderDistance() * RAY_DISTANCE_COEF) { // Live distance
                ray->kill();
            } else if (ray->getIntensity() <= LIGHT_INTENSITY_LIMIT) { // Too low intensity
                ray->kill();
            }
        }
        if (depth > 0) delete ray;
        raytracer.adv(); // Update advencement
    }

    // Rec for the clonned ones
    if (raysClones.size() > 0) {
        raytracer.advAddMax(raysClones.size()); // Update advencement max
        processLightChunk(raytracer, raysClones, 0, raysClones.size(), objects, objectsChunks, camera, depth + 1);
    }
}

static hot void processCameraChunk(raytracer::Raytracer& raytracer,
    std::vector<raytracer::Ray*>& rays, std::size_t start, std::size_t end,
    const std::vector<raytracer::IObject*>& objects,
    const std::unordered_map<raytracer::Chunk, std::vector<raytracer::IObject*>, raytracer::ChunkHash>& objectsChunks,
    raytracer::ICamera* camera, const raytracer::Sky& sky,
    raytracer::FColor globalLightColor, std::size_t globalLightCount,
    std::size_t depth = 0)
{
    std::vector<raytracer::Ray*> raysClones;
    raytracer::FColor color = DEFAULT_COLOR;
    raytracer::IObject* nearestObject = nullptr;
    raytracer::Type distanceUnit = 0.0;
    const raytracer::IMaterial* material = nullptr;
    const raytracer::Face* faceHit = nullptr;
    float t = 0.0f;

    for (std::size_t i = start; i < end; ++i) {
        raytracer::Ray* ray = rays[i];
        distanceUnit = ray->getCFrame().look.length();
        while (ray->isAlive()) {
            // Kill those with no direction
            if (ray->getCFrame().look <= 1e-8 && ray->getCFrame().look >= -1e-8) {
                ray->kill();
                continue;
            }

            // Update the different hits
            ray->computeObjects(camera->getRenderDistance(), objects, objectsChunks);

            // 1 - Compute T
            t = 0.0f;
            faceHit = nullptr;
            nearestObject = nullptr;
            for (const auto &[object, actualT, face]: ray->getHits()) {
                if (ray->getImmunity() == object) continue;
                if (!nearestObject || actualT < t) {
                    t = actualT;
                    faceHit = face;
                    nearestObject = object;
                }
            }
            if (!nearestObject) { // No valid T
                ray->setColor(raytracer::mergeColor(ray->getColor(), sky.getColor(), ray->getCoef()));
                ray->kill();
                continue;
            }

            // 2 - Apply T (aproximative gravity curve, only in newton mode)
            ray->translate(ray->getCFrame().look * t);
            ray->addDistance(distanceUnit * t);

            // Kill conditions
            if (std::isnan(t) || (ray->getCFrame().position - camera->getCFrame().position).lengthSquared() >= camera->getRenderDistance() * camera->getRenderDistance()) { // Too far
                ray->setColor(raytracer::mergeColor(ray->getColor(), sky.getColor(), ray->getCoef()));
                ray->kill();
                continue;
            } else if (ray->getDistance() >= camera->getRenderDistance() * RAY_DISTANCE_COEF) { // Live distance
                ray->kill();
                continue;
            } else if (ray->getCoef() <= CAMERA_COEF_LIMIT) { // Too low intensity
                ray->kill();
                continue;
            }

            // 3 - Check T
            material = nearestObject->getObjectDescriptor().material;
            // Transparency & Refraction
            if (material->getTransparency() > 1e-8) {
                raysClones.push_back(static_cast<raytracer::Ray*>(ray->clone()));
                raysClones.back()->setImmunity(nearestObject);
                raysClones.back()->setCoef(raysClones.back()->getCoef() * material->getTransparency() * (1.0f - material->getReflection()));
                raysClones.back()->setColor(raytracer::mergeColor(material->getColor(), raysClones.back()->getColor(), raysClones.back()->getCoef()));
            }

            // Normal computing
            float d = (ray->getCFrame().position - ray->getCFrameOrigin().position).length() / camera->getRenderDistance();
            float localIntensityCoef = std::exp(-EXP_K * d * d * d * d);
            auto [pointColor, ok] = nearestObject->getPointColor(ray->getCFrame().position);
            color = pointColor;

            // Apply the global light modifier
            if (globalLightCount > 0 && !ok) color = raytracer::mergeColor(color, raytracer::mergeLight(material->getColor(), globalLightColor, globalLightCount));
            else if (globalLightCount > 0) color = raytracer::moyColor(color, raytracer::mergeLight(material->getColor(), globalLightColor, globalLightCount));

            // Generate the noise
            if (material->hasNoise()) {
                auto [strength, size] = material->getNoiseSettings();
                raytracer::noise(ray->getCFrame().position - nearestObject->getCFrame().position, color, strength, size);
            }

            // Set the color
            color = raytracer::mergeColor(ray->getColor(), color, ray->getCoef() * localIntensityCoef);
            ray->setColor(color);

            // Update coef
            ray->setCoef(ray->getCoef() * material->getReflection());

            // Apply the reflection
            nearestObject->reflectRay(ray, faceHit);

            // To counter collision with the same object on the next iteration
            ray->setImmunity(nearestObject);

            // Kill conditions
            if (std::isnan(t) || (ray->getCFrame().position - camera->getCFrame().position).lengthSquared() >= camera->getRenderDistance() * camera->getRenderDistance()) { // Too far
                ray->setColor(raytracer::mergeColor(ray->getColor(), sky.getColor(), ray->getCoef()));
                ray->kill();
            } else if (ray->getDistance() >= camera->getRenderDistance() * RAY_DISTANCE_COEF) { // Live distance
                ray->kill();
            } else if (ray->getCoef() <= CAMERA_COEF_LIMIT) { // Too low intensity
                ray->kill();
            }
        }
        if (depth > 0) delete ray;
        raytracer.adv(); // Update advencement
    }

    // Rec for the clonned ones
    if (raysClones.size() > 0) {
        raytracer.advAddMax(raysClones.size()); // Update advencement max
        processCameraChunk(raytracer, raysClones, 0, raysClones.size(), objects, objectsChunks, camera, sky, globalLightColor, globalLightCount, depth + 1);
    }
}

/*
 1 - Reset rays (lights)
 2 - Compute lights rays
    1 - Compute T
    2 - Apply T (and aproximative gravity curve, only in newton mode)
    3 - Check
    too low intensity -> kill
    too far -> kill
    collision ->
        - Reflect
        - Reduce intensity
        - Apply color fusion
        - Add light rays data (hit point, intensity, color) to the object
*/
void raytracer::Raytracer::light(void)
{
    std::vector<std::thread> threads;
    std::size_t countThreads = 1, chunkSize = 1;
    std::size_t start = 0, end = 0;

    // 1 - Reset rays (lights)
    for (raytracer::ILight* light: this->_lights)
        light->reset();
    for (raytracer::IObject* object: this->_objects)
        object->clearLightData();

    // 2 - Compute lights rays
    this->advNext(0);
    for (raytracer::ILight* light: this->_lights) this->advAddMax(light->getRays().size());
    this->adv(true, false);
    for (raytracer::ILight* light: this->_lights) {
        if (light->isGlobal()) { // Handle global light
            this->_globalLightColor += light->getColor() * light->getIntensity();
            ++this->_globalLightCount;
            continue;
        }
        std::vector<raytracer::LightRay*> lightRays = light->getRays();
        countThreads = ((this->_settings.nproc) ? this->_settings.nproc : std::thread::hardware_concurrency()) - 1;
        chunkSize = lightRays.size() / countThreads;
        threads.clear();
        for (std::size_t i = 0; i < countThreads; ++i) {
            start = i * chunkSize;
            end = (i == countThreads - 1) ? lightRays.size() : start + chunkSize;

            // Start the thread
            threads.emplace_back(processLightChunk,
                std::ref(*this),
                std::ref(lightRays), start, end,
                std::cref(this->_objects),
                std::cref(this->_objectsChunks),
                this->_camera,
                0
            );
        }

        // Wait for the threads
        for (std::thread& t: threads)
            t.join();
    }
    this->adv(true, false);
    this->advFull();
    this->advEnd();
    this->advNext(0);
}

/*
 1 - Reset rays (camera)
 2.0 - Check for already computed ray
 2 - Compute camera rays
    1 - Compute T
    2 - Apply T (and aproximative gravity curve, only in newton mode)
    3 - Check
    too low intensity -> kill
    too far -> kill
    collision ->
        - Reflect
        - Reduce intensity
        - Apply color fusion
    too low intensity -> kill
    too far -> kill
 2.2 - Store computed value
*/
void raytracer::Raytracer::render(void)
{
    std::vector<std::thread> threads;
    std::vector<raytracer::Ray*> aliveRays;
    aliveRays.reserve(this->_camera->getRays().size());
    std::size_t countThreads = 1, chunkSize = 1;
    std::size_t start = 0, end = 0;

    // 1 - Reset rays (camera)
    this->_camera->reset();

    // 2.0 - Check for already computed ray
    for (raytracer::Ray* ray: this->_camera->getRays()) {
        const raytracer::CFrame& cframe = ray->getCFrameOrigin();
        auto it = this->_rays.find(cframe);
        if (it != this->_rays.end()) {
            ray->setColor(it->second);
            ray->kill();
        } else {
            aliveRays.push_back(ray);
        }
    }

    // 2.1 - Compute camera rays
    countThreads = ((this->_settings.nproc) ? this->_settings.nproc : std::thread::hardware_concurrency()) - 1;
    chunkSize = aliveRays.size() / countThreads;
    threads.clear();
    this->advReset(aliveRays.size());
    this->adv(true, false);
    for (std::size_t i = 0; i < countThreads; ++i) {
        start = i * chunkSize;
        end = (i == countThreads - 1) ? aliveRays.size() : start + chunkSize;

        // Start the thread
        threads.emplace_back(processCameraChunk,
            std::ref(*this),
            std::ref(aliveRays), start, end,
            std::cref(this->_objects),
            std::cref(this->_objectsChunks),
            this->_camera, std::cref(this->_sky),
            this->_globalLightColor, this->_globalLightCount,
            0
        );
    }
 
    // Wait for the threads
    for (std::thread& t: threads)
        t.join();
    this->adv(true, false);
    this->advFull();

    // End of the advencement display
    if (!this->_settings.gui)
        this->advEnd();

    // 2.2 - Store computed value
    for (raytracer::Ray* ray: aliveRays) {
        const raytracer::CFrame& cframe = ray->getCFrameOrigin();
        this->_rays.try_emplace(cframe, ray->getColor());
    }
}

cold void raytracer::Raytracer::loadRender(void)
{
    // Init default camera (to use default camera comportement)
    this->_camera = new raytracer::Viewer();

    // Try to open the ppm file
    std::ifstream file(this->_settings.ppm_path, std::ios::binary);
    if (!file)
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::ppmFile, std::string("Failed to open file for reading: ") + this->_settings.ppm_path);

    // Reading header
    std::uint8_t magic;
    std::uint16_t width, height;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.read(reinterpret_cast<char*>(&width), sizeof(width));
    file.read(reinterpret_cast<char*>(&height), sizeof(height));
    if (!file)
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::ppmFile, std::string("Error during the file header reading: ") + this->_settings.ppm_path);

    // Check magic byte
    if (magic != PPM_MAGIC)
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::ppmFile, std::string("Invalid magic number: ") + this->_settings.ppm_path);

    // Set the resolution
    this->_camera->setResolution({width, height});
    this->_camera->init(); // Reload rays
    this->_camera->kill(); // Kill all rays

    // Read pixels
    std::vector<uint8_t> buffer(width * height * 3);
    file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    std::vector<raytracer::Ray*> rays = this->_camera->getRays();
    for (std::size_t i = 0; i < width * height; ++i)
        rays[i]->setColor({buffer[i * 3 + 0], buffer[i * 3 + 1], buffer[i * 3 + 2]});

    // Check if the reading was successfull
    if (!file)
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::ppmFile, std::string("Error during the file reading: ") + this->_settings.ppm_path);
}

cold void raytracer::Raytracer::saveRender(void)
{
    // Update screen pixels using rays color
    this->_camera->updateScreen();

    // Setup the file path
    std::filesystem::path cfg = this->_settings.cfg_path;
    std::filesystem::path base = this->_settings.rendered_path;
    cfg.replace_extension(".ppm");
    std::filesystem::path finalPath;
    if (!base.empty() && base.has_extension()) { // File path given
        finalPath = base;
    } else { // Directory path given
        std::filesystem::path dir = base;
        finalPath = dir / cfg.filename();
    }

    // Create directory if needed & try to open the ppm file
    if (!finalPath.parent_path().empty())
        std::filesystem::create_directories(finalPath.parent_path());
    std::string path = finalPath.string();
    std::ofstream file(path, std::ios::binary);
    if (!file)
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::ppmFile, std::string("Failed to open file for writing: ") + path);

    // Setup header vars
    std::uint8_t magic = PPM_MAGIC;
    raytracer::Resolution resolution = this->_camera->getResolution();

    // Write file header
    file.write(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.write(reinterpret_cast<char*>(&resolution.x), sizeof(resolution.x));
    file.write(reinterpret_cast<char*>(&resolution.y), sizeof(resolution.y));

    // Write pixels
    const std::vector<raytracer::Color>& pixels = this->_camera->getScreen();
    std::vector<std::uint8_t> buffer;
    buffer.reserve(pixels.size() * 3);
    buffer.resize(pixels.size() * 3);
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        buffer[i * 3 + 0] = pixels[i].x;
        buffer[i * 3 + 1] = pixels[i].y;
        buffer[i * 3 + 2] = pixels[i].z;
    }
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());

    // Check if the writing was successfull
    if (!file)
        throw utils::exception::CustomException(utils::exception::Type::Error, utils::exception::Code::ppmFile, std::string("Error during the file writing: ") + path);
}

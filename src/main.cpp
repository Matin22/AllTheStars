#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>
#include <algorithm>

#include "events.hpp"
#include "config.hpp"
#include "star.hpp"

std::vector<Star> createStars(uint32_t count, float scale)
{
    std::vector<Star> stars;
    stars.reserve(count);

    // generate random
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    // Star free zone

    sf::Vector2f const window_world_size = conf::fWINDOW_SIZE * conf::near;
    sf::FloatRect const star_free_zone = {-window_world_size * 0.5f, window_world_size};

    // create stars on screen
    for (uint32_t i {count}; i--;)
    {
        float const x = (dis(gen) - 0.5f) * conf::fWINDOW_SIZE.x * scale;
        float const y = (dis(gen) - 0.5f) * conf::fWINDOW_SIZE.y * scale;
        float const z = dis(gen) * (conf::far - conf::near) + conf::near;

        if (star_free_zone.contains({x, y}))
        {
            i++;
            continue;
        }

        stars.push_back({{x, y}, z});
    }

    std::sort(stars.begin(), stars.end(), [](const Star& a, const Star& b)
    {
        return a.z > b.z;
    });

    return stars;
}

int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({conf::WINDOW_SIZE.x, conf::WINDOW_SIZE.y}), "CMake SFML Project", conf::window_state);
    window.setFramerateLimit(conf::MAX_FRAMERATE);

    std::vector<Star> stars = createStars(conf::count, conf::far);

    while (window.isOpen())
    {
        processEvents(window);

        uint32_t first = 0;
        // traverse effect
        for (uint32_t i{0}; i < conf::count; i++)
        {
            Star &s = stars[i];
            s.z -= conf::speed * conf::dt;

            if (s.z < conf::near)
            {
                // offset stars and make continous motion
                s.z = conf::far - (conf::near - s.z);
                first = i;
            }
        }

        window.clear();
        // rendering
        sf::CircleShape shape{conf::radius};
        shape.setOrigin(sf::Vector2f{conf::radius, conf::radius});
        for (uint32_t i{conf::count}; i--;)
        {
            uint32_t idx = (i + first) % conf::count;
            Star const &s = stars[idx];
            
            float const scale = 1.0f / s.z;
            shape.setPosition(s.position * scale + conf::fWINDOW_SIZE * 0.5f);
            shape.setScale(sf::Vector2f{scale, scale});
            
            float const depth_ratio = (s.z - conf::near) / (conf::far - conf::near);
            float const color_ratio = 1.0f - depth_ratio;
            auto const c = static_cast<uint8_t>(color_ratio * 255.0f);
            shape.setFillColor({c, c, c});
            
            window.draw(shape);
        }
        
        window.display();
    }
}

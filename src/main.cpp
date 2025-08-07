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

void updateGeometry(uint32_t idx, Star const& s, sf::VertexArray& va)
{
    float const scale = 1.0f / s.z;
    float const depth_ratio = (s.z - conf::near) / (conf::far - conf::near);
    float const color_ratio = 1.0f - depth_ratio;
    auto const c = static_cast<uint8_t>(color_ratio * 255.0f);
    sf::Color color(c, c, c);

    sf::Vector2f const p = s.position * scale;
    float const r = conf::radius * scale;
    uint32_t const i = 6 * idx;
    
    va[i + 0].position = sf::Vector2f(p.x - r, p.y - r);
    va[i + 1].position = sf::Vector2f(p.x + r, p.y - r);
    va[i + 2].position = sf::Vector2f(p.x - r, p.y + r);
    va[i + 3].position = sf::Vector2f(p.x + r, p.y - r);
    va[i + 4].position = sf::Vector2f(p.x + r, p.y + r);
    va[i + 5].position = sf::Vector2f(p.x - r, p.y + r);

    va[i + 0].color = color;
    va[i + 1].color = color;
    va[i + 2].color = color;
    va[i + 3].color = color;
    va[i + 4].color = color;
    va[i + 5].color = color;
}

int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({conf::WINDOW_SIZE.x, conf::WINDOW_SIZE.y}), "AllTheStars", conf::window_state);
    window.setFramerateLimit(conf::MAX_FRAMERATE);
    // window.setMouseCursorVisible(false);

    sf::Texture texture;
    texture.setSmooth(true);
    if (!texture.loadFromFile("res/star.png"))
    {
        std::cerr << "Failed to load star.png" << std::endl;
        return -1;
    }
    if (!texture.generateMipmap())
    {
        std::cerr << "Failed to generate mipmap" << std::endl;
    }

    std::vector<Star> stars = createStars(conf::count, conf::far);

    sf::VertexArray va(sf::PrimitiveType::Triangles, 6 * conf::count);

    // prefill texture coords -> constant
    auto const texture_size_f = static_cast<sf::Vector2f>(texture.getSize());
    for (uint32_t idx{conf::count}; idx--;)
    {
        uint32_t const i = 6 * idx;
        
        va[i + 0].texCoords = sf::Vector2f(0, 0);
        va[i + 1].texCoords = sf::Vector2f(texture_size_f.x, 0);
        va[i + 2].texCoords = sf::Vector2f(0, texture_size_f.y);

        va[i + 3].texCoords = sf::Vector2f(texture_size_f.x, 0);
        va[i + 4].texCoords = sf::Vector2f(texture_size_f.x, texture_size_f.y);
        va[i + 5].texCoords = sf::Vector2f(0, texture_size_f.y);
    }

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

        for (uint32_t i{conf::count}; i--;)
        {
            uint32_t idx = (i + first) % conf::count;
            Star const &s = stars[idx];
            updateGeometry(i, s, va);
        }

        sf::RenderStates states;
        states.transform.translate(conf::fWINDOW_SIZE * 0.5f);
        states.texture = &texture;
        window.draw(va, states);

        window.display();
    }
}

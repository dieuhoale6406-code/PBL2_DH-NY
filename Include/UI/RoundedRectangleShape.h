// RoundedRectangleShape.h
#pragma once
#include <SFML/Graphics.hpp>
#include <math.h>

namespace sf
{
    class RoundedRectangleShape : public sf::Shape
    {
    public:
        explicit RoundedRectangleShape(const sf::Vector2f& size = sf::Vector2f(0, 0), float radius = 0, unsigned int cornerPointCount = 10);
        void setSize(const sf::Vector2f& size);
        const sf::Vector2f& getSize() const;
        void setCornersRadius(float radius);
        float getCornersRadius() const;
        void setCornerPointCount(unsigned int count);
        virtual std::size_t getPointCount() const;
        virtual sf::Vector2f getPoint(std::size_t index) const;

    private:
        sf::Vector2f m_size;
        float m_radius;
        unsigned int m_cornerPointCount;
    };

    inline RoundedRectangleShape::RoundedRectangleShape(const sf::Vector2f& size, float radius, unsigned int cornerPointCount)
    {
        m_size = size;
        m_radius = radius;
        m_cornerPointCount = cornerPointCount;
        update();
    }

    inline void RoundedRectangleShape::setSize(const sf::Vector2f& size)
    {
        m_size = size;
        update();
    }

    inline const sf::Vector2f& RoundedRectangleShape::getSize() const
    {
        return m_size;
    }

    inline void RoundedRectangleShape::setCornersRadius(float radius)
    {
        m_radius = radius;
        update();
    }

    inline float RoundedRectangleShape::getCornersRadius() const
    {
        return m_radius;
    }

    inline void RoundedRectangleShape::setCornerPointCount(unsigned int count)
    {
        m_cornerPointCount = count;
        update();
    }

    inline std::size_t RoundedRectangleShape::getPointCount() const
    {
        return m_cornerPointCount * 4;
    }

    inline sf::Vector2f RoundedRectangleShape::getPoint(std::size_t index) const
    {
        if (index >= m_cornerPointCount * 4)
            return sf::Vector2f(0, 0);

        float deltaAngle = 90.0f / (m_cornerPointCount - 1);
        sf::Vector2f center;
        unsigned int centerIndex = index / m_cornerPointCount;
        static const float pi = 3.141592654f;

        switch (centerIndex)
        {
        case 0: center.x = m_size.x - m_radius; center.y = m_radius; break;
        case 1: center.x = m_radius; center.y = m_radius; break;
        case 2: center.x = m_radius; center.y = m_size.y - m_radius; break;
        case 3: center.x = m_size.x - m_radius; center.y = m_size.y - m_radius; break;
        }

        return sf::Vector2f(m_radius * cos(deltaAngle * (index - centerIndex) * pi / 180) + center.x,
            -m_radius * sin(deltaAngle * (index - centerIndex) * pi / 180) + center.y);
    }
}
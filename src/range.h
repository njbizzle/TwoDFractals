#ifndef RANGE_H
#define RANGE_H

class range {
public:
    float min;
    float max;

    bool flip;
    bool clamp;

    float size;

    range(float min_, float max_)
    {
        min = min_;
        max = std::max(min_, max_);
        flip = false;
        clamp = false;

        size = max - min;
    }

    range(float min_, float max_, bool flip_, bool clamp_)
    {
        min = min_;
        max = std::max(min_, max_);
        flip = flip_;
        clamp = clamp_;

        size = max - min;
    }

    int minI()
    {
        return static_cast<int>(min);
    }

    int maxI()
    {
        return static_cast<int>(max);
    }

    float clampNum(float x)
    {
        return std::min(std::max(static_cast<float>(x), min), max);
    }

    // map from this to [-1,1]
    float normalize(float x)
    {
        if (clamp)
        {
            clampNum(x);
        }

        if (flip)
        {
            return 1 - ((x - min) / size);
        }

        return (x - min) / size;
    }

    // map from this to other
    float mapTo(range other, float x)
    {
        return other.denormalize(normalize(x));;
    }

    // map from [0,1] to this
    float denormalize(float x)
    {
        if (flip)
        {
            x = 1 - x;
        }

        if (clamp)
        {
            return clampNum((x * size) + min);
        }
        return (x * size) + min;
    }

    void setSize(float newSize, float scalePoint)
    {
        if (!contains(scalePoint)){return;}

        // this bit took far too much thinking

        float p = (scalePoint - min) / size;
        float s = (newSize - size);

        min -= p * s;
        max += (1 - p) * s;
        size = max - min;
    }

    bool contains(float x)
    {
        return (min < x && x < max);
    }

    range& operator*=(float scalar)
    {
        min -= scalar;
        max += scalar;
        size = max - min;
        return *this;
    }

    range& operator+=(float scalar)
    {
        min += scalar * size;
        max += scalar * size;
        return *this;
    }

};

class rangeTwoD
{
public:
    range x;
    range y;

    rangeTwoD(range x_, range y_): x(x_), y(y_){};
};

bool clampedVector2(sf::Vector2f vec, rangeTwoD a)
{
    return a.x.contains(vec.x) && a.y.contains(vec.y);
}

sf::Vector2f clampVector2(sf::Vector2f vec, rangeTwoD a)
{
    vec.x = a.x.clampNum(vec.x);
    vec.y = a.y.clampNum(vec.y);

    return vec;
}

Eigen::Matrix3f transformToMatrix3f(rangeTwoD i, rangeTwoD f)
{
    // took a lot of thinking but im proud of this

    Eigen::Matrix3f m = Eigen::Matrix3f::Zero();

    float xScale = f.x.size / i.x.size;
    float yScale = f.y.size / i.y.size;

    if (i.x.flip == f.x.flip)
    {
        m(0,0) = xScale;
        m(0, 2) = f.x.min - i.x.min * xScale;
    }
    else
    {
        m(0,0) = -xScale;
        m(0, 2) = f.x.max + i.x.min * xScale;
    }


    if (i.y.flip == f.y.flip)
    {
        m(1,1) = yScale;
        m(1, 2) = f.y.min - i.y.min * yScale;
    }
    else
    {
        m(1,1) = -yScale;
        m(1, 2) = f.y.max + i.y.min * yScale;
    }

    m(2,2) = 1; // for homogenous coordinates (must remain 1)

    return m;
}

// handle homogenous coords
sf::Vector2f transformPoint(const Eigen::Matrix3f& matrix, const sf::Vector2f& point) {
    Eigen::Vector3f vec(point.x, point.y, 1.0f);
    Eigen::Vector3f transformed = matrix * vec;

    if (transformed.z() != 0.0f) {
        transformed.x() /= transformed.z();
        transformed.y() /= transformed.z();
    }
    return sf::Vector2f(transformed.x(), transformed.y());
}

#endif

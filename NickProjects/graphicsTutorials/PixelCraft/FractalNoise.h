#pragma once
#include <FastNoise/FastNoise.h>
#include <memory>

class FractalNoise {
private:
    // Store the actual FastNoise::SmartNode objects
    FastNoise::SmartNode<FastNoise::Simplex> fnSimplex;
    FastNoise::SmartNode<FastNoise::FractalRidged> fractalNode;
    float m_frequency;
    int m_seed;

public:
    // Constructor that sets up the noise structure once
    FractalNoise(float frequency, float persistence, int octaves, int seed)
        : m_frequency(frequency), m_seed(seed)
    {
        fnSimplex = FastNoise::New<FastNoise::Simplex>();
        fractalNode = FastNoise::New<FastNoise::FractalRidged>();
        fractalNode->SetSource(fnSimplex);
        fractalNode->SetOctaveCount(octaves);
        fractalNode->SetGain(persistence);
        fractalNode->SetLacunarity(2.0f);
    }

    // Default constructor for container initialization
    FractalNoise()
        : m_frequency(0.0f), m_seed(0)
    {
        fnSimplex = FastNoise::New<FastNoise::Simplex>();
        fractalNode = FastNoise::New<FastNoise::FractalRidged>();
        fractalNode->SetSource(fnSimplex);
    }

    // Efficient method to get noise at a specific position
    float getNoise2D(int worldX, int worldY) const {
        return fractalNode->GenSingle2D(worldX * m_frequency, worldY * m_frequency, m_seed);
    }

};

#ifndef PLANT_THRESHOLDS_H
#define PLANT_THRESHOLDS_H

// Enum definine
enum PlantType {
    SPIDER_PLANT,
    MONSTERA,
    SNAKE_PLANT,
    PEACE_LILY,
    ALOE_VERA,
    FIDDLE_LEAF_FIG,
    ENGLISH_IVY,
    POTHOS,
    CALATHEA,
    RUBBER_PLANT,
    NUM_PLANTS // Total number of plant types
};

// Plant names
static const char* PlantNames[NUM_PLANTS] = {
    "Spider Plant",
    "Monstera",
    "Snake Plant",
    "Peace Lily",
    "Aloe Vera",
    "Fiddle Leaf",
    "English Ivy",
    "Pothos",
    "Calathea",
    "Rubber Plant"
};

// Class for environmental threshold values for each plant type
class PlantThreshold {
public:
    float min_temp;   // Minimum temperature in °C
    float max_temp;   // Maximum temperature in °C
    float min_humidity;  // Minimum relative humidity (%)
    float max_humidity;  // Maximum relative humidity (%)
    float min_soil;   // Minimum soil moisture (%)
    float max_soil;   // Maximum soil moisture (%)
    float daily_light_required; // Daily light requirement 
    float min_light;  // Minimum instantaneous light threshold (0–100 scale)
    float max_light;  // Maximum instantaneous light threshold (0–100 scale)

    // Constructor to initialize all the threshold values for a plant type
    PlantThreshold(float t_min, float t_max, float h_min, float h_max,
                   float s_min, float s_max, float daily_light,
                   float l_min, float l_max)
        : min_temp(t_min), max_temp(t_max),
          min_humidity(h_min), max_humidity(h_max),
          min_soil(s_min), max_soil(s_max),
          daily_light_required(daily_light),
          min_light(l_min), max_light(l_max) {}
};

// Static array containing threshold values for all 10 plant types.
static const PlantThreshold PlantDatabase[NUM_PLANTS] = {
    {18, 27, 30, 60, 50, 60, 720, 30, 90},   // Spider Plant
    {18, 27, 60, 80, 30, 60, 900, 40, 100},  // Monstera
    {15, 29, 30, 50, 0, 25, 500, 20, 80},    // Snake Plant
    {18, 26, 60, 80, 35, 65, 800, 30, 85},   // Peace Lily
    {13, 27, 20, 40, 0, 25, 600, 30, 90},    // Aloe Vera
    {20, 28, 50, 60, 30, 60, 1000, 50, 100}, // Fiddle Leaf Fig
    {15, 25, 50, 70, 30, 60, 700, 30, 85},   // English Ivy
    {18, 27, 40, 60, 25, 55, 750, 35, 95},   // Pothos
    {18, 24, 60, 90, 35, 65, 850, 30, 85},   // Calathea
    {18, 27, 40, 60, 25, 50, 850, 35, 90}    // Rubber Plant
};

#endif // PLANT_THRESHOLDS_H

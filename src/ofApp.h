#pragma once

#include "ofMain.h"
#include <array>

struct fly
{
    std::array<size_t, 4> position;
    float fitness;
};

//-------------------------------------------
class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
    
private:
    
    std::vector<fly> population;
    const std::size_t population_size = 100;
    const std::size_t image_dim = 50;
    
    ofImage surface;
    ofImage best_1, best_2;
    size_t index_best_fly;
    float disturbance_threshold = 0.3;

};

#include "ofApp.h"

struct Color{
    float R,G,B,X,Y,Z,L,a,b;
};

#define REF_X 95.047; // Observer= 2°, Illuminant= D65
#define REF_Y 100.000;
#define REF_Z 108.883;

Color rgb2xyz(int R,int G,int B){
    float r = R / 255.0;
    float g = G / 255.0;
    float b = B / 255.0;
    
    if (r > 0.04045){ r = pow((r + 0.055) / 1.055, 2.4); }
    else { r = r / 12.92; }
    if ( g > 0.04045){ g = pow((g + 0.055) / 1.055, 2.4); }
    else { g = g / 12.92; }
    if (b > 0.04045){ b = pow((b + 0.055) / 1.055, 2.4); }
    else {  b = b / 12.92; }
    
    r = r * 100;
    g = g * 100;
    b = b * 100;
    //Observer. = 2°, Illuminant = D65
    Color xyz;
    xyz.X = r * 0.4124 + g * 0.3576 + b * 0.1805;
    xyz.Y = r * 0.2126 + g * 0.7152 + b * 0.0722;
    xyz.Z = r * 0.0193 + g * 0.1192 + b * 0.9505;
    return xyz;
}
Color xyz2lab(float X,float Y, float Z){
    float x = X / REF_X;
    float y = Y / REF_X;
    float z = Z / REF_X;
    
    if ( x > 0.008856 ) { x = pow( x , .3333333333f ); }
    else { x = ( 7.787 * x ) + ( 16/116.0 ); }
    if ( y > 0.008856 ) { y = pow( y , .3333333333f ); }
    else { y = ( 7.787 * y ) + ( 16/116.0 ); }
    if ( z > 0.008856 ) { z = pow( z , .3333333333f ); }
    else { z = ( 7.787 * z ) + ( 16/116.0 ); }
    
    Color lab;
    lab.L = ( 116 * y ) - 16;
    lab.a = 500 * ( x - y );
    lab.b = 200 * ( y - z );
    return lab;
}
Color lab2xyz(float l, float a, float b){
    float y = (l + 16) / 116;
    float x = a / 500 + y;
    float z = y - b / 200;
    
    if ( pow( y , 3 ) > 0.008856 ) { y = pow( y , 3 ); }
    else { y = ( y - 16 / 116 ) / 7.787; }
    if ( pow( x , 3 ) > 0.008856 ) { x = pow( x , 3 ); }
    else { x = ( x - 16 / 116 ) / 7.787; }
    if ( pow( z , 3 ) > 0.008856 ) { z = pow( z , 3 ); }
    else { z = ( z - 16 / 116 ) / 7.787; }
    
    Color xyz;
    xyz.X = x * REF_X;
    xyz.Y = y * REF_Y;
    xyz.Z = z * REF_Z;
    return xyz;
}
Color xyz2rgb(float X,float Y,float Z){
    //X from 0 to  95.047      (Observer = 2°, Illuminant = D65)
    //Y from 0 to 100.000
    //Z from 0 to 108.883
    X = ofClamp(X, 0, 95.047);
    
    float x = X * .01;
    float y = Y * .01;
    float z = Z * .01;
    
    float r = x * 3.2406 + y * -1.5372 + z * -0.4986;
    float g = x * -0.9689 + y * 1.8758 + z * 0.0415;
    float b = x * 0.0557 + y * -0.2040 + z * 1.0570;
    
    if ( r > 0.0031308 ) { r = 1.055 * pow( r , ( 1 / 2.4f ) ) - 0.055; }
    else { r = 12.92 * r; }
    if ( g > 0.0031308 ) { g = 1.055 * pow( g , ( 1 / 2.4f ) ) - 0.055; }
    else { g = 12.92 * g; }
    if ( b > 0.0031308 ) { b = 1.055 * pow( b , ( 1 / 2.4f ) ) - 0.055; }
    else { b = 12.92 * b; }
    
    Color rgb;
    rgb.R = round( r * 255 );
    rgb.G = round( g * 255 );
    rgb.B = round( b * 255 );
    return rgb;
}
Color rgb2lab(int R,int G,int B){
    Color xyz = rgb2xyz(R, G, B);
    return xyz2lab(xyz.X, xyz.Y, xyz.Z);
}
Color lab2rgb(int L,int a,int b){
    Color xyz = lab2xyz(L, a, b);
    return xyz2rgb(xyz.X, xyz.Y, xyz.Z);
}

float dist_lab(Color c1, Color c2){
    float dL = c1.L - c2.L;
    float da = c1.a - c2.a;
    float db = c1.b - c2.b;
    return sqrt(dL*dL + da*da + db*db);
}

size_t get_random(float size)
{
    return size_t(std::round(ofRandom(1.0f) * size));
}

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(12);
    
    surface.load("7.png");
    
    best_1.allocate(image_dim, image_dim, OF_IMAGE_COLOR);
    best_2.allocate(image_dim, image_dim, OF_IMAGE_COLOR);
    
    //----------------------------------------------------------
    // assign random position & fill array
    //----------------------------------------------------------
    
    population.reserve(population_size);
    for (size_t i = 0; i < population_size; ++i)
    {
        fly f;
        f.position = { get_random(surface.getWidth() - image_dim * 2) + image_dim, get_random(surface.getHeight() - image_dim * 2) + image_dim, get_random(surface.getWidth() - image_dim * 2) + image_dim, get_random(surface.getHeight() - image_dim * 2) + image_dim };
        population.push_back(f);
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    
    const ofRectangle boundaries = ofRectangle(0, 0, ofGetWidth(), ofGetHeight());

    float best_fitness = std::numeric_limits<float>::max();
    size_t index = 0;
    
    for (auto& f : population)
    {
        const size_t fly_x = f.position[0];
        const size_t fly_y = f.position[1];
        const size_t fly_delta_x = f.position[2];
        const size_t fly_delta_y = f.position[3];
        const ofPoint point_1(fly_x + fly_delta_x, fly_y + fly_delta_y);
        const ofPoint point_2(fly_x - fly_delta_x, fly_y - fly_delta_y);
        
        if (boundaries.inside(point_1.x + image_dim, point_1.y + image_dim) &&
            boundaries.inside(point_2.x - image_dim, point_2.y - image_dim))
        {
            f.fitness = 0;
            for (size_t x = 0; x < image_dim; ++x)
            {
                for (size_t y = 0; y < image_dim; ++y)
                {
                    const float brightness_1 = surface.getColor(fly_x + fly_delta_x + x, fly_y + fly_delta_y + y).getBrightness();
                    const float brightness_2 = surface.getColor(fly_x - fly_delta_x - x, fly_y - fly_delta_y - y).getBrightness();
                    
                    
                    ofColor c1 = surface.getColor(fly_x + fly_delta_x + x, fly_y + fly_delta_y + y);
                    ofColor c2 = surface.getColor(fly_x - fly_delta_x - x, fly_y - fly_delta_y - y);
                    Color lab_1 = rgb2lab(c1.r, c1.g, c1.b);
                    Color lab_2 = rgb2lab(c2.r, c2.g, c2.b);
                    f.fitness += dist_lab(lab_1, lab_2);
                }
            }
        }
        else
        {
            f.fitness = std::numeric_limits<float>::max();
        }
        

        if (f.fitness < best_fitness)
        {
            best_fitness = f.fitness;
            index_best_fly = index;
            
            for (size_t x = 0; x < image_dim; ++x)
            {
                for (size_t y = 0; y < image_dim; ++y)
                {
                    best_1.setColor(x, y, surface.getColor(fly_x + fly_delta_x + x, fly_y + fly_delta_y + y));
                    best_2.setColor(x, y, surface.getColor(fly_x - fly_delta_x - x, fly_y - fly_delta_y - y));
                }
            }
            best_1.update();
            best_2.update();
            cout << fly_x + fly_delta_x << ' ' << fly_y + fly_delta_y << std::endl;
            cout << fly_x + fly_delta_x << ' ' << fly_y + fly_delta_y << std::endl;
        }
        ++index;
    }
    
    index = 0;
    size_t left = 0;
    size_t right = 0;
    
    for (auto& f : population)
    {
        if (index != index_best_fly)
        {
            if (index == 0)
            {
                left = population.size() - 1;
                right = index + 1;
            }
            else if (index == population.size() - 1)
            {
                left = index - 1;
                right = 0;
            }
            else
            {
                left = index - 1;
                right = index + 1;
            }
            
            fly best_neigbour = (population[left].fitness < population[right].fitness) ? population[left] : population[right];
            
            if (ofRandom(1.0) < disturbance_threshold)
            {
                f.position = { get_random(surface.getWidth() - image_dim * 2) + image_dim, get_random(surface.getHeight() - image_dim * 2) + image_dim, get_random(surface.getWidth() - image_dim * 2) + image_dim, get_random(surface.getHeight() - image_dim * 2) + image_dim };
            }
            else
            {
                for (size_t i = 0; i < 4; ++i)
                    f.position[i] = best_neigbour.position[i] + ofRandom(1.0) * (population[index_best_fly].position[i] - best_neigbour.position[i]);
            }
        }
        ++index;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    //----------------------------------------------------------
    // display the image
    //----------------------------------------------------------
    ofSetColor(255);
    surface.draw(0, 0);
    
    //----------------------------------------------------------
    // display the flies
    //----------------------------------------------------------
    ofSetColor(0);
    for (auto f : population){
        ofDrawCircle(f.position[0], f.position[1], 2);
    }
    
    //----------------------------------------------------------
    // display the best fly
    //----------------------------------------------------------
    ofSetColor(255, 0, 0);
    ofDrawCircle(population[index_best_fly].position[0], population[index_best_fly].position[1], 4);
    
    ofSetColor(255, 255);
    best_1.draw(0, surface.getHeight());
    best_2.draw(image_dim * 2, surface.getHeight());
}


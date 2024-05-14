#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
int max(int a,int b){
    return (((a) > (b)) ? (a) : (b));
}
//
//  main.cpp
//  CGP
//
//  Created by Garv Grover on 14/04/24.
//
#include <GLUT/glut.h>
#include <iostream>
#include <cmath> // Add this line for cos and sin functions
#include "stb_image.h"

// Window dimensions
int screenWidth = 800;
int screenHeight = 600;

// Position of the man and the balloon
float manX = screenWidth / 2.0f;
float manY = screenHeight / 2.0f;
float balloonX = 50.0f;
float balloonY = screenHeight - 50.0f;

bool balloonPopped = false; // Flag to indicate if the balloon is popped

// Ground dimensions
float groundWidth = screenWidth;
float groundHeight = 50.0f;
float groundY = 0.0f;

// Marked landing area dimensions
float landingAreaWidth = 200.0f;
float landingAreaHeight = 20.0f;
float landingAreaX = (screenWidth - landingAreaWidth) / 2.0f;
float landingAreaY = groundHeight;

int lives = 3; // Number of lives
int score = 0; // Score variable
int maxscore=0;
int level=1;

float balloonVelocityX = 2.0f;

bool gameover=false;
bool startScreen = true;

GLuint backgroundTexture;
GLuint groundTexture;
GLuint gameoverTexture;

// Load the image and generate the texture
// Function to load a texture and return the texture ID
// Function to load a texture and return the texture ID
GLuint loadTexture(const char* imagePath) {
    int width, height, channels;
    unsigned char* imageData = stbi_load(imagePath, &width, &height, &channels, STBI_rgb_alpha); // Load with alpha channel

    if (imageData == NULL) {
        std::cerr << "Error: Could not load the image from path: " << imagePath << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Determine the correct format (GL_RGBA for images with an alpha channel)
    GLenum format = GL_RGBA;

    // Upload the image data to the texture
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);

    // Set texture filtering and wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Free the image data after uploading it to the GPU
    stbi_image_free(imageData);

    return textureID;
}


void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1); // Set the coordinate system
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    backgroundTexture=loadTexture("sky.jpg");
    groundTexture=loadTexture("ground.jpg");
    gameoverTexture=loadTexture("ngameover.png");
}

void drawMan() {
    // Draw the man as a simple stick figure for now
    glColor3f(1.0f, 1.0f, 1.0f); // White color
    glLineWidth(3.0f);
    
    glBegin(GL_LINES);
        glVertex2f(manX, manY); // Body
        glVertex2f(manX, manY - 40.0f);
        glVertex2f(manX, manY - 10.0f); // Arms
        glVertex2f(manX - 20.0f, manY - 20.0f);
        glVertex2f(manX, manY - 10.0f);
        glVertex2f(manX + 20.0f, manY - 20.0f);
        glVertex2f(manX, manY - 40.0f); // Legs
        glVertex2f(manX - 10.0f, manY - 60.0f);
        glVertex2f(manX, manY - 40.0f);
        glVertex2f(manX + 10.0f, manY - 60.0f);
    glEnd();
}

void drawBalloon() {
    // Draw the balloon as a simple circle for now
    glColor3f(0.11f, 0.53f, 0.12f); // Green color
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.14159f / 180.0f;
        float x = balloonX + cos(angle) * 20.0f;
        float y = balloonY + sin(angle) * 20.0f;
        glVertex2f(x, y);
    }
    glEnd();
}

// Function to draw the ground
void drawGround() {
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    // Bind the ground texture
    glBindTexture(GL_TEXTURE_2D, groundTexture);

    // Draw a quad covering the ground portion of the screen
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, 0.0f); // Bottom-left corner

        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(screenWidth, 0.0f); // Bottom-right corner

        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(screenWidth, groundHeight); // Top-right corner

        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, groundHeight); // Top-left corner
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}


void drawLandingArea() {
    glColor3f(0.53f, 0.12f, 0.11f); // Red color
    glBegin(GL_QUADS);
        glVertex2f(landingAreaX, landingAreaY); // Bottom-left
        glVertex2f(landingAreaX + landingAreaWidth, landingAreaY); // Bottom-right
        glVertex2f(landingAreaX + landingAreaWidth, landingAreaY + landingAreaHeight); // Top-right
        glVertex2f(landingAreaX, landingAreaY + landingAreaHeight); // Top-left
    glEnd();
}

bool isInsideLandingArea(float x, float y) {
    // Check if the given coordinates (x, y) are inside the landing area
    return x >= landingAreaX && x <= landingAreaX + landingAreaWidth &&
           y >= landingAreaY && y <= landingAreaY + landingAreaHeight;
}

bool isOutsideLandingArea(float x, float y){
    return (x < landingAreaX  || x > landingAreaX + landingAreaWidth) &&
           y >= landingAreaY && y <= landingAreaY + landingAreaHeight;
}

void drawScore() {
    glColor3f(1.0f, 1.0f, 1.0f); // White color
    glRasterPos2f(10, screenHeight - 20); // Position of the text
    std::string scoreText = "Score: " + std::to_string(score);
    for (char c : scoreText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); // Render each character of the score
    }
    glRasterPos2f(screenWidth-50, screenHeight - 20); // Position of the text
    std::string livesText = "Lives: " + std::to_string(lives);
    for (char c : livesText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); // Render each character of the score
    }
}

void updateLandingArea() {
    // Decrease the width of the landing area
    landingAreaWidth -= 0.15f; // Adjust the rate of shrinking as needed
    landingAreaX = (screenWidth - landingAreaWidth) / 2.0f;
    // Ensure that the width doesn't go below zero
    if (landingAreaWidth < 0) {
        landingAreaWidth = 0;
    }

    // Check if the landing area width is zero
    if (landingAreaWidth == 0) {
        // Game over condition
        std::cout << "Game over! Landing area size reached zero." << std::endl;
        gameover=true;
        // Implement game over logic here, such as displaying a game over message
    }
}

void drawStartScreen() {
    glColor3f(1.0f, 1.0f, 1.0f); // White color
    glRasterPos2f(screenWidth / 2 - 100, screenHeight / 2);
    std::string startText = "Press spacebar to start";
    for (char c : startText) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c); // Render each character of the start message
    }
}

void drawGameOverScreen() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    // Bind the ground texture
    glBindTexture(GL_TEXTURE_2D, gameoverTexture);

    // Draw a quad  covering the ground portion of the screen
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(100.0f, 320.0f); // Bottom-left corner

        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(screenWidth-100.0, 320.0f); // Bottom-right corner

        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(screenWidth-100.0, screenHeight-30.0f); // Top-right corner

        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(100.0f, screenHeight-30.0f); // Top-left corner
    glEnd();

    
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glRasterPos2f(screenWidth / 2-20, screenHeight / 2 - 60);
    std::string sscore = "Score: "+ std::to_string(score);
    for (char c : sscore) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c); // Render each character of the restart message
    }
    maxscore=max(maxscore,score);
    glRasterPos2f(screenWidth / 2-40, screenHeight / 2 - 90);
    std::string mscore = "Max Score: "+ std::to_string(maxscore);
    for (char c : mscore) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c); // Render each character of the restart message
    }
}

// Function to draw the sky background
void drawBackground() {
    glEnable(GL_TEXTURE_2D);

    // Bind the sky texture
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);

    // Draw a quad covering the entire screen
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, screenHeight); // Top-left corner

        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(screenWidth, screenHeight); // Top-right corner

        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(screenWidth, 0.0f); // Bottom-right corner

        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 0.0f); // Bottom-left corner
    glEnd();

    glDisable(GL_TEXTURE_2D);
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawBackground();
    if(startScreen){
        drawStartScreen();
    }
    else if(gameover){
        drawGameOverScreen();
    }
    else{
        // Draw the scene
        drawMan();
        drawBalloon();
        drawGround();
        drawLandingArea();
        drawScore(); // Display the score

        // Check for collision with the landing area
        if(balloonPopped){
            if (isInsideLandingArea(manX, manY)) {
                // Increase the score if the man lands inside the landing area
                score++;
                std::cout << "Score: " << score << std::endl;
                
                // Reset the man's position to the initial position
                balloonX = 50.0f;
                balloonY = screenHeight - 50.0f;
                manX = balloonX;
                manY = balloonY - 20.0f;
                balloonPopped=false;
                balloonVelocityX+=0.5f;
                landingAreaWidth = 200.0f;
            }
            if (isOutsideLandingArea(manX, manY)){
                lives--;
                std::cout<< "Lives: "<<lives<<std::endl;
                if(lives==0){
                    std::cout<<"Game Over"<<std::endl;
                    gameover=true;
                }
                
                balloonX = 50.0f;
                balloonY = screenHeight - 50.0f;
                manX = balloonX;
                manY = balloonY - 20.0f;
                balloonPopped=false;
                landingAreaWidth = 200.0f;
            }
        }
        updateLandingArea();
    }

    glutSwapBuffers();
}



void handleKeypress(unsigned char key, int x, int y) {
    if (startScreen) {
        // If the game is in the start screen state
        if (key == 32) { // Spacebar pressed
            startScreen = false; // Start the game
        }
    }
    else if(gameover){
        if(key=='r'){
            gameover=false;
            lives=3;
            score=0;
            balloonX = 50.0f;
            balloonY = screenHeight - 50.0f;
            manX = balloonX;
            manY = balloonY - 20.0f;
            balloonPopped = false;
            landingAreaWidth = 200.0f;
            landingAreaHeight = 20.0f;
            landingAreaX = (screenWidth - landingAreaWidth) / 2.0f;
            landingAreaY = groundHeight;
            balloonVelocityX = 2.0f;
            
        }
    }
    else {
        // If the game is running
        switch (key) {
            case 32: // Spacebar pressed
                std::cout << "Spacebar pressed. Before setting balloonPopped: " << balloonPopped << std::endl;
                balloonPopped = true;
                std::cout << "After setting balloonPopped: " << balloonPopped << std::endl;
                break;
            case 'r': // 'R' pressed
                gameover = false; // Restart the game
                break;
            case 27: // ASCII code for escape key
                exit(0);
                break;
        }
    }
}

void update(int value) {
    // Update the position of the balloon
    balloonX += balloonVelocityX;

    // Update the position of the man to match the balloon's position
    manX = balloonX;

    // Check for collision with the edges of the screen
    if (balloonX <= 0 || balloonX >= screenWidth) {
        // Reverse the velocity to make the balloon bounce
        balloonVelocityX *= -1;
    }

    if (balloonPopped) {
        // Implement falling animation here
//        manAngle += windSpeed;
//        manY -= (balloonY + sin(manAngle) * 40.0f)-10.0f;
        manY -= 10.0f;
        // Example: decrease man's y-coordinate by 10 units per frame
//        std::cout << "ManY after update: " << manY << std::endl;
    }

    glutPostRedisplay(); // Call display function
    glutTimerFunc(16, update, 0); // 60 fps
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Balloon Jumper");

    initGL();
    manX = balloonX; // Move the man slightly to the right of the balloon
    manY = balloonY - 20.0f;
    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeypress);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}

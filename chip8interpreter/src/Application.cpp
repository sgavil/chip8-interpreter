#include <iostream>
#include "chip8.h"

#include <GL/glut.h>

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;

void renderScene(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glutSwapBuffers();
}


int main(int argc, char** argv) {
	chip8 _chip8;

	//if (argc < 2)
	//{
	//	printf("Usage: myChip8.exe chip8application\n\n");
	//	return 1;
	//}

	//// Load game
	//if (!_chip8.load_application(argv[1]))
	//	return 1;

	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(320, 320);
	glutCreateWindow("chip8interpreter - sgavil");

	// register callbacks
	glutDisplayFunc(renderScene);

	// enter GLUT event processing cycle
	glutMainLoop();

	return 0;

}
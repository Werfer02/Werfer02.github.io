#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_Image.h>
#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>

#undef main

#define WIN_CENTERED SDL_WINDOWPOS_CENTERED //didnt fit nicely otherwise also why the long name

#define OUT_OF_BOUNDS_ID -1
#define EMPTY_ID 0
#define SAND_ID 1
#define WATER_ID 2
#define WALL_ID 3

#define NONE_PHYS_ID -1
#define STATIC_PHYS_ID 0
#define SAND_PHYS_ID 1
#define LIQUID_PHYS_ID 2



class SimPixelFlavourMap { //i really dont know how else to do this
public:
	std::map<int, SDL_Color> colorMap{

		{EMPTY_ID ,SDL_Color(0,   0,   0,  255)},
		{SAND_ID,  SDL_Color(249, 205, 71, 255)},
		{WATER_ID, SDL_Color(40,  77,  249,255)},
		{WALL_ID,  SDL_Color(111, 131, 132,255)}

	};

	std::map<int, int> physicsMap{

		{OUT_OF_BOUNDS_ID , NONE_PHYS_ID},
		{EMPTY_ID , NONE_PHYS_ID},
		{SAND_ID,  SAND_PHYS_ID},
		{WATER_ID, LIQUID_PHYS_ID},
		{WALL_ID, STATIC_PHYS_ID}

	};

	std::map<int, int> densityMap{

		{OUT_OF_BOUNDS_ID , 99999},
		{EMPTY_ID , 0},
		{SAND_ID,  2},
		{WATER_ID, 1},
		{WALL_ID, 100}

	};

};

class SimPixel {
public:

	int flavourID;
	bool selected;
	bool isUI;

	SimPixel(int flavID) {

		flavourID = flavID;
		selected = false;
		isUI = false;

	}

};

class Grid { 
public:
	int rows; //rows of rects ("pixels")
	int columns; //columns
	int width; //width in pixels
	int height;	//height in pixels
	int rectWidthInPixels;
	int rectHeightInPixels;
	int colsCenter;
	int rowsCenter;
	bool colsIsOdd;
	bool rowsIsOdd;
	SimPixelFlavourMap SPFM;
	std::vector<std::vector<SimPixel>> simPixelGrid;
	std::vector<std::vector<SDL_Point>> rectPosGrid;

	
	Grid(int r, int c, int w, int h, SimPixelFlavourMap SPFM) {

		rows = r, columns = c, width = w, height = h, SPFM = SPFM; //constructor

		rectWidthInPixels = int(width / columns); //divide the width evenly between the columns and cast to int

		rectHeightInPixels = int(height / rows); //divide the height evenly between the rows and cast to int

		colsCenter = int(columns / 2); //cast to int to round down bc floor returns a double for some reason

		rowsCenter = int(rows / 2); 

		colsIsOdd = columns % 2;

		rowsIsOdd = rows % 2;

		for (int i = 0; i < rows; i++) { 

			std::vector<SimPixel> rowVector; //vector for each row bc i cant think of any other way to do this

			for (int j = 0; j < columns; j++) { 

				SimPixel gridPixel(EMPTY_ID);

				rowVector.push_back(gridPixel); //hope this works how i think it does

			}

			simPixelGrid.push_back(rowVector);

		}

	}

	SDL_Rect GenWholeGridRect(int winWidth, int winHeight) {

		int leftEdge = (winWidth / 2) - (width / 2);
		int topEdge = (winHeight / 2) - (height / 2); //self explanatory

		SDL_Rect WholeGridRect = SDL_Rect(leftEdge, topEdge, width, height); 

		return WholeGridRect;

	}

SDL_Point GenRectPos(int rectRow, int rectCol, int winWidth, int winHeight, int space) {
    SDL_Point genPoint;

    int screenCenterWidth = winWidth / 2;
    int screenCenterHeight = winHeight / 2;

    int tileWidthWithSpace = rectWidthInPixels + space;
    int tileHeightWithSpace = rectHeightInPixels + space;

    genPoint.x = (screenCenterWidth - (((colsCenter + 1) - rectCol) * tileWidthWithSpace)) - (tileWidthWithSpace / 2) * colsIsOdd;
    genPoint.y = (screenCenterHeight - (((rowsCenter + 1) - rectRow) * tileHeightWithSpace)) - (tileHeightWithSpace / 2) * rowsIsOdd;

    return genPoint;
}


	void GenRectPosVector(int winWidth, int winHeight, int space) {

		SDL_Point genPoint;
		std::vector<std::vector<SDL_Point>> genVec;

		int screenCenterWidth = winWidth / 2;

		int screenCenterHeight = winHeight / 2;

		int tileWidthWithSpace = rectWidthInPixels + space;
		int tileHeightWithSpace = rectHeightInPixels + space;

		for (int i = 0; i < rows; i++) {

			std::vector<SDL_Point> rowVector; //copied from constructor

			for (int j = 0; j < columns; j++) {

				genPoint.x = (screenCenterWidth - (((colsCenter + 1) - j) * tileWidthWithSpace)) - (tileWidthWithSpace / 2) * colsIsOdd;
				genPoint.y = (screenCenterHeight - (((rowsCenter + 1) - i) * tileHeightWithSpace)) - (tileHeightWithSpace / 2) * rowsIsOdd;

				rowVector.push_back(genPoint); //hope this works how i think it does

			}

			genVec.push_back(rowVector);

		}

		rectPosGrid = genVec;

	}

	SDL_Point ScreenPointToGridPoint(int winWidth, int winHeight, SDL_Point& screenPoint) {

		int leftEdge = (winWidth / 2) - (width / 2);
		int topEdge = (winHeight / 2) - (height / 2); //get x and y offset

		int gridPointX = screenPoint.x - leftEdge; 
		int gridPointY = screenPoint.y - topEdge; //take offset away

		SDL_Point gridPoint; //make point to return
		gridPoint.x = gridPointX;
		gridPoint.y = gridPointY;

		return gridPoint;
	}

	SimPixel& GridPointToSimPixel(SDL_Point& gridPoint) {

		int colIndex = int(gridPoint.x / rectWidthInPixels); //self explanatory
		int rowIndex = int(gridPoint.y / rectHeightInPixels);

		return simPixelGrid.at(rowIndex).at(colIndex); //never made a function that returns a reference before its pretty cool tbh

	}

	SDL_Point GridPointToSimPixelIndex(SDL_Point& gridPoint) {

		SDL_Point simPixelIndex;

		simPixelIndex.x = int(gridPoint.x / rectWidthInPixels); //self explanatory
		simPixelIndex.y = int(gridPoint.y / rectHeightInPixels);

		return simPixelIndex; 

	}

	void ClearSimPixelTempAttributes() { // theres 10000000% a better way to do this

		for (int i = 1; i <= simPixelGrid.at(0).size(); i++) {

			for (int j = 1; j <= simPixelGrid.size(); j++) {

				simPixelGrid.at(j - 1).at(i - 1).selected = false;
				simPixelGrid.at(j - 1).at(i - 1).isUI = false;

			}

		}

	}

	bool GridPointInGrid(SDL_Point& gridPoint) {

		if (gridPoint.x > 0 && gridPoint.x < width) {

			if (gridPoint.y > 0 && gridPoint.y < height) {

				return true;

			}
			else return false;

		}
		else return false;
	}

	bool SimPixelIndexInGrid(SDL_Point& simPixelIndex) {

		if (simPixelIndex.x >= 0 && simPixelIndex.x < columns) {

			if (simPixelIndex.y >= 0 && simPixelIndex.y < rows) {

				return true;

			}
			else return false;

		}
		else return false;
	}

	int GetSimPixelFlavourIDAtIndexSafe(SDL_Point& simPixelIndex) {

		if (SimPixelIndexInGrid(simPixelIndex)) {

			return simPixelGrid.at(simPixelIndex.y).at(simPixelIndex.x).flavourID;

		}
		else {
			return OUT_OF_BOUNDS_ID;
		}

	}

	std::vector<SDL_Point> ReturnLine(SDL_Point simPixelStart, SDL_Point simPixelEnd) {

		std::vector<SDL_Point> simPixelIndexVector;

		int dx = simPixelEnd.x - simPixelStart.x; //x distance
		int dy = simPixelEnd.y - simPixelStart.y; //y distance

		int steps = std::max(std::abs(dx), std::abs(dy)); //steps

		float xStep = 0;
		float yStep = 0; 

		if (steps != 0) { //divide by zero bad

			xStep = static_cast<float>(dx) / steps; //x per step
			yStep = static_cast<float>(dy) / steps; //y per step //had to static cast float because it looked shit because of the lost precision otherwise

		}

		for (int i = 0; i <= steps; i++) {
			
			SDL_Point simPixelIndex;

			int x = std::round(simPixelStart.x + i * xStep); // ith step x
			int y = std::round(simPixelStart.y + i * yStep); // ith step y

			simPixelIndex.x = x;
			simPixelIndex.y = y;

			simPixelIndexVector.push_back(simPixelIndex);

		}

		return simPixelIndexVector;

	}

	void MoveSimPixelFlavour(SDL_Point& originSimPixelIndex, int amountX, int amountY) {

			simPixelGrid.at(originSimPixelIndex.y + amountY).at(originSimPixelIndex.x + amountX).flavourID = simPixelGrid.at(originSimPixelIndex.y).at(originSimPixelIndex.x).flavourID; //copy flavour over

			simPixelGrid.at(originSimPixelIndex.y).at(originSimPixelIndex.x).flavourID = EMPTY_ID; //empty original

	}

	void SwapSimPixelFlavour(SDL_Point& originSimPixelIndex, int amountX, int amountY) {

		int copyFlavour = simPixelGrid.at(originSimPixelIndex.y).at(originSimPixelIndex.x).flavourID; //store flavour to move

		simPixelGrid.at(originSimPixelIndex.y).at(originSimPixelIndex.x).flavourID = simPixelGrid.at(originSimPixelIndex.y + amountY).at(originSimPixelIndex.x + amountX).flavourID; //change original

		simPixelGrid.at(originSimPixelIndex.y + amountY).at(originSimPixelIndex.x + amountX).flavourID = copyFlavour; //copy flavour over

	}

};

void DrawGrid(SDL_Renderer* renderer, Grid& grid, SDL_Color& cursorCol, SDL_Color& uiColor, int wWidth, int wHeight) {

	SDL_Rect gridRect = SDL_Rect(0, 0, 0, 0);

	SDL_Color gridColor = SDL_Color(0, 0, 0, 255); //set default colour to black

	for (int i = 1; i <= grid.columns; i++) {
		for (int j = 1; j <= grid.rows; j++) {

			SDL_Color simPixelColor = grid.SPFM.colorMap[grid.simPixelGrid.at(j - 1).at(i - 1).flavourID]; //simpixelflavourmap colourmap indicated color of the flavor of current simpixel

			if (grid.simPixelGrid.at(j - 1).at(i - 1).selected) { //j and i minus one because the vector starts at 0 but grid starts at 1

				gridColor = cursorCol; //if the pixel is selected set it to cursorCol


				SDL_SetRenderDrawColor(renderer, gridColor.r, gridColor.g, gridColor.b, gridColor.a); //set color
			}
			else if (grid.simPixelGrid.at(j - 1).at(i - 1).isUI) {

				gridColor = uiColor;

				SDL_SetRenderDrawColor(renderer, gridColor.r, gridColor.g, gridColor.b, gridColor.a);
			}
			else {
				SDL_SetRenderDrawColor(renderer, simPixelColor.r, simPixelColor.g, simPixelColor.b, simPixelColor.a); //set color
			}

			SDL_Point genRectPos = grid.rectPosGrid.at(j - 1).at(i - 1);

			gridRect = SDL_Rect(genRectPos.x, genRectPos.y, grid.rectWidthInPixels, grid.rectHeightInPixels); //generate a rect to draw using the methods from grid class

			SDL_RenderFillRect(renderer, &gridRect); //draw the rect

		}
	}

}

void ChangeFlavourOfSelectedSimPixels(Grid& grid, int& flavourID) {

	for (int i = 1; i <= grid.columns; i++) {
		for (int j = 1; j <= grid.rows; j++) {

			if (grid.simPixelGrid.at(j - 1).at(i - 1).selected == true) { //j and i minus one because the vector starts at 0 but grid starts at 1

				grid.simPixelGrid.at(j - 1).at(i - 1).flavourID = flavourID;

			}

		}
	}

}

void SelectCursor(Grid& grid, SDL_Point simPixelIndex, int cursorSize) {
	
	if (cursorSize <= 1) { //if cursor size is 1 or less just draw a dot

		if (grid.SimPixelIndexInGrid(simPixelIndex)) {

			grid.simPixelGrid.at(simPixelIndex.y).at(simPixelIndex.x).selected = true;

		}

	}
	else {
		for (int i = 0; i < cursorSize; i++) {

			SDL_Point drawSimPixel;
			drawSimPixel.y = simPixelIndex.y;

			drawSimPixel.x = simPixelIndex.x - i; //1 to the left

			if (grid.SimPixelIndexInGrid(drawSimPixel)) {
				grid.simPixelGrid.at(drawSimPixel.y).at(drawSimPixel.x).selected = true;
			}

			drawSimPixel.x = simPixelIndex.x + i; //1 to the right

			if (grid.SimPixelIndexInGrid(drawSimPixel)) {
				grid.simPixelGrid.at(drawSimPixel.y).at(drawSimPixel.x).selected = true;
			}

			drawSimPixel.x = simPixelIndex.x;

			drawSimPixel.y = simPixelIndex.y + i; //1 to the up

			if (grid.SimPixelIndexInGrid(drawSimPixel)) {
				grid.simPixelGrid.at(drawSimPixel.y).at(drawSimPixel.x).selected = true;
			}

			drawSimPixel.y = simPixelIndex.y - i; //1 to the down

			if (grid.SimPixelIndexInGrid(drawSimPixel)) {
				grid.simPixelGrid.at(drawSimPixel.y).at(drawSimPixel.x).selected = true;
			}

		}
	}
}

void MarkSlider(Grid& grid, SDL_Point& startSimPixelIndex, int size, char axis) {

	if (axis == 'x') {

		for (int i = 0; i < size; i++) {

			grid.simPixelGrid.at(startSimPixelIndex.y).at(startSimPixelIndex.x + i).isUI = true;

		}

	}
	else if (axis == 'y') {

		for (int i = 0; i < size; i++) {

			grid.simPixelGrid.at(startSimPixelIndex.y + i).at(startSimPixelIndex.x).isUI = true;

		}

	}

}

void PhysicsStep(Grid& grid) {

	SDL_Point currentSimPixelIndex;

	SDL_Point movedSimPixelIndex;

	int waterSpreadRate = 12;
	int sandSpreadRate = 3;

	auto moveLoop = [](Grid& grid, int times, int amountDown, int randomVar, SDL_Point& originIndex) -> bool { //what the FUCK is a lambda

		SDL_Point movedIndex;
		movedIndex.x = 0;
		movedIndex.y = 0;



		if (grid.GetSimPixelFlavourIDAtIndexSafe(originIndex) == SAND_ID) { //sand move loop

			for (int i = 1; i <= times; i++) {

				movedIndex = originIndex;

				movedIndex.y += i; //the difference is sand only checks diagonally
				movedIndex.x += randomVar * i; //diagonally to the right or left

				if (grid.GetSimPixelFlavourIDAtIndexSafe(movedIndex) == EMPTY_ID) {

					grid.MoveSimPixelFlavour(originIndex, randomVar * i, i); //move random right or left and down

					return true;

				}
				else if (grid.GetSimPixelFlavourIDAtIndexSafe(movedIndex) != grid.GetSimPixelFlavourIDAtIndexSafe(originIndex)) { //it can go through itself thats fine but dont let it go through other stuff

					return false;

				}


			}

		}
		else if (grid.GetSimPixelFlavourIDAtIndexSafe(originIndex) == WATER_ID) { //water move loop

			for (int i = 1; i <= times; i++) {

				movedIndex = originIndex;

				movedIndex.y += amountDown;
				movedIndex.x += randomVar * i; //down and to the right or left

				if (grid.GetSimPixelFlavourIDAtIndexSafe(movedIndex) == EMPTY_ID) {

					grid.MoveSimPixelFlavour(originIndex, randomVar * i, amountDown); //move random right or left and down

					return true;

				}
				else if (grid.GetSimPixelFlavourIDAtIndexSafe(movedIndex) != grid.GetSimPixelFlavourIDAtIndexSafe(originIndex)) { //it can go through itself thats fine but dont let it go through other stuff

					return false;

				}


			}

		}

		return false;

	};


	for (int i = grid.columns; i >= 1; i--) { //iterate backwards
		for (int j = grid.rows; j >= 1; j--) {

			currentSimPixelIndex.y = j - 1;
			currentSimPixelIndex.x = i - 1;

			if (grid.SPFM.physicsMap[grid.simPixelGrid.at(currentSimPixelIndex.y).at(currentSimPixelIndex.x).flavourID] != NONE_PHYS_ID) { //skip over things like empty or out of bounds if that ever happens

				if (grid.SPFM.physicsMap[grid.simPixelGrid.at(currentSimPixelIndex.y).at(currentSimPixelIndex.x).flavourID] == SAND_PHYS_ID) { //sand physics

					SDL_Point movedSimPixelIndex = currentSimPixelIndex;

					movedSimPixelIndex.y += 1;

					if (grid.GetSimPixelFlavourIDAtIndexSafe(movedSimPixelIndex) == EMPTY_ID) { //if empty below and in bounds

						grid.MoveSimPixelFlavour(currentSimPixelIndex, 0, 1); //move down

					}
					else if (grid.SPFM.densityMap[grid.GetSimPixelFlavourIDAtIndexSafe(movedSimPixelIndex)] < grid.SPFM.densityMap[grid.GetSimPixelFlavourIDAtIndexSafe(currentSimPixelIndex)]) {

						grid.SwapSimPixelFlavour(currentSimPixelIndex, 0, 1); //swap down

					}
					else {

						float randomMinusOneToOne = (std::rand() % 2); //i want 2 unique ints 

						int randomMoveDir = randomMinusOneToOne < 0.5 ? -1 : 1; //if less than .5 equal to -1 else equal to 1

						movedSimPixelIndex = currentSimPixelIndex;

						bool movedDiagonally = moveLoop(grid, sandSpreadRate, 1, randomMoveDir, currentSimPixelIndex);
					}
				}
				else if (grid.SPFM.physicsMap[grid.simPixelGrid.at(currentSimPixelIndex.y).at(currentSimPixelIndex.x).flavourID] == LIQUID_PHYS_ID) { //water physics

					movedSimPixelIndex = currentSimPixelIndex;

					movedSimPixelIndex.y += 1;

					if (grid.GetSimPixelFlavourIDAtIndexSafe(movedSimPixelIndex) == EMPTY_ID) {

						grid.MoveSimPixelFlavour(currentSimPixelIndex, 0, 1); //move down

					}
					else if (grid.SPFM.densityMap[grid.GetSimPixelFlavourIDAtIndexSafe(movedSimPixelIndex)] < grid.SPFM.densityMap[grid.GetSimPixelFlavourIDAtIndexSafe(currentSimPixelIndex)]) {

						grid.SwapSimPixelFlavour(currentSimPixelIndex, 0, 1); //swap down

					}
					else {

						float randomMinusOneToOne = (std::rand() % 2); //i want 2 unique ints 

						int randomMoveDir = randomMinusOneToOne < 0.5 ? -1 : 1; //if less than .5 equal to -1 else equal to 1

						movedSimPixelIndex = currentSimPixelIndex;

						bool movedDiagonally1 = moveLoop(grid, waterSpreadRate, 1, randomMoveDir, currentSimPixelIndex);

						bool movedHorizontally = moveLoop(grid, waterSpreadRate, 0, randomMoveDir, currentSimPixelIndex);

					}


				}
				else if (grid.SPFM.physicsMap[grid.simPixelGrid.at(currentSimPixelIndex.y).at(currentSimPixelIndex.x).flavourID] == STATIC_PHYS_ID) {

					//nothing here but this should be considered because its not NONE_PHYS_ID but STATIC_PHYS_ID
					//(trust me theres a difference)

				}

			}

		}
	}
}



int main() {

	int winHeight = 720, winWidth = 1280;

	int cursorSize = 5;

	int frameStart = 0, frameEnd = 0;
	float fps = 0;

	bool quit = false;
	bool clicking = false;
	bool simRunning = true;

	int selectedFlavour = SAND_ID;
	int cursorFlavour = EMPTY_ID;

	SDL_Init(SDL_INIT_EVERYTHING); //hi SDL

#pragma region creating stuff and checks

	SimPixelFlavourMap testSPFM;

	Grid testGrid(200, 200, 600, 600, testSPFM); //100x100 rows and columns 400x400 pixels //remember rows and cols cant be more than pixels also width and height have to be divisible by rows and cols unfortunately

	SDL_Rect gridRect = testGrid.GenWholeGridRect(winWidth, winHeight);

	SDL_Color cursorCol = SDL_Color(0, 255, 165, 255);

	SDL_Color uiCol = SDL_Color(239, 116, 24, 255);

	SDL_Event windowEvent;

	SDL_Point cursorSliderStart(2, 2);
	
	SDL_Point testPoint1(60, 60);

	SDL_Point mousePos;
							//initialize all of these so they dont throw stupid errors
	SDL_Point gridMousePosNew;

	SDL_Point gridMousePosOld;

	SDL_Surface* derry = SDL_LoadBMP("\image1.bmp");

	SDL_Window* window = SDL_CreateWindow("test", WIN_CENTERED, WIN_CENTERED, winWidth, winHeight, 0);

	if (window == nullptr) {
		std::cout << "Epic window fail " << SDL_GetError() << std::endl;
		SDL_Delay(5000);
		return 1;
	}

	SDL_Renderer* windowRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); //window, index, 0 so default flags (accelerated)

	if (windowRenderer == nullptr) {
		std::cout << "Epic renderer fail " << SDL_GetError() << std::endl;
		SDL_Delay(5000);
		return 1;
	}

	SDL_Texture* derrytexture = SDL_CreateTextureFromSurface(windowRenderer, derry);

	SDL_FreeSurface(derry);

#pragma endregion 

	SDL_RenderClear(windowRenderer); //fill render with draw colour

	testGrid.GenRectPosVector(winWidth, winHeight, 0);

	while (!quit) {

		frameStart = SDL_GetTicks();

		SDL_RenderCopy(windowRenderer, derrytexture, NULL, NULL);

		testGrid.ClearSimPixelTempAttributes();

#pragma region <------- Input

		while (SDL_PollEvent(&windowEvent)) {
			
			if (windowEvent.motion.x > 1 || windowEvent.motion.y > 1) { //scrolling for some reason changes motion to -1 or 1

				mousePos.x = windowEvent.motion.x;
				mousePos.y = windowEvent.motion.y;

			}

			//NOTE: got rid of if statement checking if mousepos is in the grid because selectcursor already checks and it allows selecting the cursor if mouse is outside grid

			gridMousePosNew = testGrid.ScreenPointToGridPoint(winWidth, winHeight, mousePos); //get which simpixel to change

			switch (windowEvent.type) {

				case SDL_QUIT: quit = true; //if x clicked

				case SDL_KEYDOWN:

					if (windowEvent.key.keysym.sym == SDLK_1) { //if 1 pressed

						selectedFlavour = SAND_ID;

					}
					if (windowEvent.key.keysym.sym == SDLK_2) {

						selectedFlavour = WATER_ID;

					}
					if (windowEvent.key.keysym.sym == SDLK_3) {

						selectedFlavour = WALL_ID;

					}
					if (windowEvent.key.keysym.sym == SDLK_SPACE) {

						simRunning = !simRunning;

					};

				case SDL_MOUSEBUTTONDOWN: //if mouse clicked 
					if (windowEvent.button.button == SDL_BUTTON_RIGHT) { //erase with right click

						cursorFlavour = EMPTY_ID;

					}
					else if (windowEvent.button.button == SDL_BUTTON_LEFT) { //draw with left click

						cursorFlavour = selectedFlavour;

					}
					clicking = true;
					;

				case SDL_MOUSEBUTTONUP: //if mouse up (apparently not)

					if (windowEvent.button.state == SDL_RELEASED) { //?????????? If mouse released?????

						clicking = false;

					};

				case SDL_MOUSEWHEEL:
					if (windowEvent.wheel.y > 0 && windowEvent.wheel.y < 20) { //if scroll up also theres some weird bug that causes clicking to set windowevent.wheel.y to somewhere around 600 i have no idea why
						
						cursorSize--;

					}
					else if (windowEvent.wheel.y < 0) { //if scroll down

						cursorSize++;

					}
					if (cursorSize < 1) {

						cursorSize = 1;

					};

			}

		}
#pragma endregion

		SelectCursor(testGrid, testGrid.GridPointToSimPixelIndex(gridMousePosNew), cursorSize); //select the simpixels that are within the cursor (do this twice so it doesnt dissapear when no input)


		if (testGrid.GridPointInGrid(gridMousePosNew) && testGrid.GridPointInGrid(gridMousePosOld)) { //if clicking in grid

			if (gridMousePosNew.x != gridMousePosOld.x || gridMousePosNew.y != gridMousePosOld.y) { //check if the points are different (if mouse input changed)

				std::vector<SDL_Point> lineSimPixelIndices = testGrid.ReturnLine(testGrid.GridPointToSimPixelIndex(gridMousePosOld), testGrid.GridPointToSimPixelIndex(gridMousePosNew)); //return line

				for (int i = 0; i < lineSimPixelIndices.size(); i++) {

					SelectCursor(testGrid, lineSimPixelIndices[i], cursorSize);

				}

			}

		}		

		if (clicking) {

			ChangeFlavourOfSelectedSimPixels(testGrid, cursorFlavour);

		}

		if (simRunning) {

			PhysicsStep(testGrid);

		}

		MarkSlider(testGrid, cursorSliderStart, cursorSize, 'x');

		DrawGrid(windowRenderer, testGrid, cursorCol, uiCol, winWidth, winHeight); //draw grid

		gridMousePosOld = gridMousePosNew;

		SDL_RenderPresent(windowRenderer); //show render

		frameEnd = SDL_GetTicks() - frameStart;

		fps = 1000 / static_cast<float>(frameEnd);

		//SDL_Delay(200);

		std::cout << fps << "\n";

	}

	SDL_DestroyTexture(derrytexture);
	SDL_DestroyRenderer(windowRenderer);  //destroy the stuff we dont need it anymore
	SDL_DestroyWindow(window);

	SDL_Quit(); //bye SDL
	return 0;

}

// NOTES
// 1: the weird formula: basically gridcenter + 1 is the box where the x is already ok at the center of the screen, then take away the column the rect is in to see how far to the left (right being negative left) it is,
// then multiply that by the width of a rect to get that distance in pixels, and take it away from the center of the screen.
// The last part is just if the amount of cols is odd take an extra half the width of a rect away so its centered.



// DAY ONE
// got SDL working
// got an image showing
// found a way to draw pixels with SDL_RenderDrawPoint

// DAY TWO
// created Grid class
// started writing some methods

// DAY THREE
// removed some testing stuff
// added the weird formula (Note 1)
// realised i dont need SDL_RenderDrawPoint anymore
// wrote some more functions and simpixel class
// setup main loop and closing window
// setup getting mouse input for later

// DAY FOUR
// Happy New Years 2024!!! 
// wrote method for getting grid coords from screen coords
// wrote method for getting which simpixel the mouse is on
// got rid of more testing stuff
// wrote some comments
// the first executable to send to mr. derry transparent
// updated drawGrid to have the option of drawing a checkerboard instead
// wrote drawCursor method

// DAY FIVE
// Fixed cursor oob error
// Wrote CreateSDLColor (same as CreateSDLRect)
// changed colors to sdl colors

// DAY SIX ?
// created simPixelFlavourMap class
// got maps working for colours

// DAY SEVEN
// a week working on this now i guess
// wrote drawline function to fix gaps in input and it looks good now
// rewrote some functions to take simpixel indices instead of gridpoints
// got cursor to work properly (drawing now draws all selected with cursor)
// considering moving some things to other files but also i forgot how to do that and its cool if its all in 1 file
// added scrolling to change cursor size also found why scrolling fucked with the cursor showing up and its worrying me
// added the possibility of drawing ui

// DAY EIGHT
// sort of got water working its just very slow and i need to implement velocity and REMEMBER TO ADD LOOKING DIAGONALLY FURTHER
// got switching flavours
// will have to look into why this is so slow

// DAY NINE
// changed from string flavour identifiers to int IDs and fps went from 25 - 30 to am inconsistent 40 - 45
// changed the diagonal and horizontal movement of water to be based on a "spread" variable and it has like 0 effect on performane somehow
// added density map and sand is denser than water (epic)
// 2nd version to be sent to derry opaque
// remember to add velocity and diagonal spreading in less dense materials

// DAY TEN 
// ten days woohoo
// added pausing with space
// added physics map so its so easy to add new flavours now
// added wall flavour accessed with 3
// will work on optimisations next time

// DAY ELEVEN
// made it so rect positions for drawing simpixels to screen are stored in their own vector and generated at the start instead of being generated every frame
// idk if i should be doing this but i changed the sdl object creation functions to constructors for those objects

// DAY TWELVE 
// preemptively checking if simpixel has no physics before checking the physics type makes fps go from 28 - 30 to around 40

// LONG BREAK

// DAY THIRTEEN 
// attempt at a transfer to GLUT
// epic fail

// DAY FOURTEEN
//
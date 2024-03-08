
#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_Image.h>
#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>
#include <SDL_ttf.h>
#include "OpenSimplexNoise.hh"

#undef main

#define RIGHT_EDGE 0
#define LEFT_EDGE 1
#define TOP_EDGE 2
#define BOTTOM_EDGE 3

#define WIN_CENTERED SDL_WINDOWPOS_CENTERED

#define TEST_ITEM_ID 0
#define TEST_ITEM_ID_2 1
#define STONE_ITEM_ID 2
#define STICKS_ITEM_ID 3

typedef const int cint;

std::map<int, const char*> itemTextureMap{

	{STONE_ITEM_ID , "rock.png"},
	{STICKS_ITEM_ID , "stick.png"},
	{TEST_ITEM_ID , "image1.bmp"},
	{TEST_ITEM_ID_2 , "guy.png"}

};
std::map<int, std::string> itemNameMap{

	{TEST_ITEM_ID , "test item"},
	{TEST_ITEM_ID_2 , "other test item"},
	{STONE_ITEM_ID , "stone"},
	{STICKS_ITEM_ID , "sticks"}

};

struct boundingBox {

	int leftBound = NULL;
	int rightBound = NULL;
	int topBound = NULL;
	int bottomBound = NULL;

	boundingBox(int inx, int iny, int inw, int inh) {

		leftBound = inx;
		rightBound = inx + inw;
		topBound = iny;
		bottomBound = iny + inh;

	}
	boundingBox(){}

};

struct betterSDLRect {

	SDL_Rect rect;
	SDL_Point originPos;
	SDL_Point originSize;

	betterSDLRect(int inx, int iny, int inw, int inh) {

		rect.x = inx, originPos.x = inx;
		rect.y = iny, originPos.y = iny;
		rect.w = inw, originSize.x = inw;
		rect.h = inh, originSize.y = inh;

		originCenterRect();
	}
	betterSDLRect() {}
	void originCenterRect() {
	
		rect.x = originPos.x - int(0.5 * rect.w);
		rect.y = originPos.y - int(0.5 * rect.h);

	}
	void centerRect() {

		rect.x = rect.x - int(0.5 * rect.w);
		rect.y = rect.y - int(0.5 * rect.h);

	}
	void growRect(int amount) {

		rect.w += amount;
		rect.h += amount;

		centerRect();
	}
	void shrinkRect(int amount) {

		rect.w -= amount;
		rect.h -= amount;

		if (rect.w < 0) { rect.w = 0; }; 
		if (rect.h < 0) { rect.h = 0; };

		centerRect();
	}
	void moveRect(int amountX, int amountY) {

		rect.x += amountX;
		rect.y += amountY;

		centerRect();

	}
	operator SDL_Rect () {

		return rect;

	}
	operator SDL_Rect*() {

		return &rect;

	}
	operator const SDL_Rect* () {

		return &rect;

	}
};

struct invItem {
	int count = 1;
	int itemID;
	std::string itemName;
	SDL_Surface* itemTextureSurface;

	invItem(cint inItemID) {

		itemID = inItemID;

		itemName = itemNameMap[itemID];

		itemTextureSurface = IMG_Load(itemTextureMap[itemID]);

	}



};

struct inventory {

	std::vector<invItem> itemArray;
	bool isActive = false;
	int invID = 0;
	int maxSize = 0;
	int maxStack = 25;

	void insertItem(invItem itemIn, int amount) {

		for (int i = 0; i < itemArray.size(); i++) {

			if (itemArray[i].itemID == itemIn.itemID) {

				if (itemArray[i].count + amount > maxStack) {

					int added = maxStack - itemArray[i].count;

					itemArray[i].count += added;

					int left = amount - added;

					int stacks = float(left) / maxStack;

					int rem = left % maxStack;

					for (int i = 0; i < stacks; i++) {

						itemArray.push_back(itemIn);
						itemArray[itemArray.size() - 1].count += maxStack - 1;

					}
					if (rem > 0) {

						itemArray.push_back(itemIn);
						itemArray[itemArray.size() - 1].count += rem - 1;

					}
					return;

				}
				else {

					itemArray[i].count += amount;
					return;

				}

			}

		}

		//if didnt add to item
		if (itemArray.size() + 1 <= maxSize) {

			int stacks = float(amount) / maxStack;

			int rem = amount % maxStack;

			for (int i = 0; i < stacks; i++) {

				itemArray.push_back(itemIn);
				itemArray[itemArray.size() - 1].count += maxStack - 1;

			}
			if (rem > 0) {

				itemArray.push_back(itemIn);
				itemArray[itemArray.size() - 1].count += rem - 1;

			}


		}
		else std::cout << "inventory full !";

	}

	void insertItemID(int itemInID, int amount) {

		for (int i = 0; i < itemArray.size(); i++) {

			if (itemArray[i].itemID == itemInID && itemArray[i].count < maxStack) {

				if (itemArray[i].count + amount > maxStack) {

					int added = maxStack - itemArray[i].count;

					itemArray[i].count += added;

					int left = amount - added;

					int stacks = float(left) / maxStack;

					int rem = left % maxStack;

					for (int i = 0; i < stacks; i++) {

						itemArray.push_back(invItem(itemInID));
						itemArray[itemArray.size() - 1].count += maxStack - 1;

					}
					if (rem > 0) {

						itemArray.push_back(invItem(itemInID));
						itemArray[itemArray.size() - 1].count += rem - 1;

					}
					return;

				}
				else {

					itemArray[i].count += amount;
					return;

				}

			}

		}

		//if didnt add to item

		if (itemArray.size() + 1 <= maxSize) {


			int stacks = float(amount) / maxStack;

			int rem = amount % maxStack;

			for (int i = 0; i < stacks; i++) {

				itemArray.push_back(invItem(itemInID));
				itemArray[itemArray.size() - 1].count += maxStack - 1;

			}
			if (rem > 0) {

				itemArray.push_back(invItem(itemInID));
				itemArray[itemArray.size() - 1].count += rem - 1;

			}


		}
		else std::cout << "inventory full !";
		

	}

};

class WorldObject {

public:

	betterSDLRect baseScreenRect;
	const char* textureName;
	SDL_Texture* texture = NULL;
	SDL_FPoint worldPos;
	SDL_FPoint worldSize;
	boundingBox colBox;
	inventory objectInv;
	int dropsID = 0;
	int dropsCount = 0;
	int maxHP = 0;
	int HP = 0;
	bool alive = true;
	bool dropsItems = false;

	WorldObject(int inx, int iny, int inw, int inh, const char* inTextureName, int inHP, int inventory, bool drops) {

		baseScreenRect = betterSDLRect(inx, iny, inw, inh);

		worldPos.x = inx, worldPos.y = iny;
		worldSize.x = inw, worldSize.y = inh;

		colBox = boundingBox(baseScreenRect.rect.x, baseScreenRect.rect.y, baseScreenRect.rect.w, baseScreenRect.rect.h);

		textureName = inTextureName;

		maxHP = inHP;
		HP = maxHP;

		dropsItems = drops;

		if (inventory) {

			objectInv.isActive = true;

			//do something idfk

		}
			
	}
	void MoveInWorld(float amountX, float amountY) {

		worldPos.x += amountX;
		worldPos.y += amountY;

		updateRect();
		updateColBox();

	}
	void updateRect() {

		baseScreenRect.rect.x = int(worldPos.x);
		baseScreenRect.rect.y = int(worldPos.y);

		baseScreenRect.centerRect();

	}
	void updateColBox() {

		colBox = boundingBox(baseScreenRect.rect.x, baseScreenRect.rect.y, baseScreenRect.rect.w, baseScreenRect.rect.h);

	}
	void damage(int amountHP) {

		HP -= amountHP;
		if (HP <= 0) {

			HP = 0;

			die();

		}

	}
	void die() {

		alive = false;

	}
	bool inBounds(boundingBox bounds) {

		if (colBox.rightBound > bounds.leftBound &&
			colBox.leftBound < bounds.rightBound &&
			colBox.topBound < bounds.bottomBound &&
			colBox.bottomBound > bounds.topBound) {

			return true;

		}
		else return false;


	}
	bool totallyInBounds(boundingBox bounds) {

		if (colBox.rightBound > bounds.leftBound &&
			colBox.leftBound > bounds.leftBound &&

			colBox.leftBound < bounds.rightBound &&
			colBox.rightBound < bounds.rightBound &&

			colBox.topBound < bounds.bottomBound &&
			colBox.bottomBound < bounds.bottomBound &&

			colBox.bottomBound > bounds.topBound &&
			colBox.topBound > bounds.topBound)
		{

			return true;

		}
		else return false;


	}
	bool pointInBounds(SDL_Point point) {

		if (point.x > colBox.leftBound &&
			point.x < colBox.rightBound &&
			point.y > colBox.topBound &&
			point.y < colBox.bottomBound) {

			return true;
		}
		else return false;


	}

};

class World {

public:
	betterSDLRect baseRect;
	std::vector <WorldObject> worldObjectArray;
	SDL_Renderer* targetRenderer; 
	boundingBox worldBounds;

	World(SDL_Renderer* inTargetRenderer, int worldWidth, int worldHeight){
	
		targetRenderer = inTargetRenderer;

		baseRect = betterSDLRect(0, 0, worldWidth, worldHeight);

		worldBounds = boundingBox(baseRect.rect.x, baseRect.rect.y, baseRect.rect.w, baseRect.rect.h);

	} 
	int insertWorldObject(WorldObject object) { 

		object.texture = IMG_LoadTexture(targetRenderer, object.textureName);

		worldObjectArray.push_back(object); 

		return worldObjectArray.size() - 1;

	}
	int objectContainingPoint(SDL_Point point) {

		for (int i = 0; i < worldObjectArray.size(); i++) {

			if (worldObjectArray[i].pointInBounds(point)) {

				return i;

			}

		}
		return -1;

	}

};

struct mineTimerParams {

	int damageHP = 0;
	WorldObject* objectToMine;
	WorldObject* objectMining;

	mineTimerParams(int dmgIn, WorldObject* objectIn, WorldObject* miningObject) {

		damageHP = dmgIn;
		objectToMine = objectIn;
		objectMining = miningObject;

	}

};

void RenderWorld(SDL_Renderer* targetRenderer, World world, SDL_Point camPos, SDL_Point camSize) {

	SDL_Point winSize;

	SDL_GetRendererOutputSize(targetRenderer, &winSize.x, &winSize.y);

	boundingBox camBounds(camPos.x, camPos.y, camSize.x, camSize.y);

	SDL_SetRenderDrawColor(world.targetRenderer, 176, 145, 33, 255);

	SDL_Rect bgRect(world.baseRect.rect.x - camPos.x + (winSize.x * 0.5) - (camSize.x * 0.5),
					world.baseRect.rect.y - camPos.y + (winSize.y * 0.5) - (camSize.y * 0.5),
					world.baseRect.rect.w,
					world.baseRect.rect.h);

	SDL_RenderFillRect(world.targetRenderer, &bgRect);

	SDL_SetRenderDrawColor(world.targetRenderer, 0, 0, 0, 255);


	for (int i = 0; i < world.worldObjectArray.size(); i++) {

		if (world.worldObjectArray[i].inBounds(camBounds)) {
			
			if (world.worldObjectArray[i].alive) {

				SDL_Rect drawRect(world.worldObjectArray[i].baseScreenRect.rect.x - camPos.x + (winSize.x * 0.5) - (camSize.x * 0.5), 
					world.worldObjectArray[i].baseScreenRect.rect.y - camPos.y + (winSize.y * 0.5) - (camSize.y * 0.5), 
					world.worldObjectArray[i].baseScreenRect.rect.w, 
					world.worldObjectArray[i].baseScreenRect.rect.h); 

				SDL_SetRenderDrawColor(world.targetRenderer, 0, 0, 0, 255); 

				//SDL_RenderDrawRect(world.targetRenderer, &drawRect); 

				SDL_RenderCopy(world.targetRenderer, world.worldObjectArray[i].texture, NULL, &drawRect); 

			}

		}

	}

}

void RenderGrid(SDL_Renderer* targetRenderer, int side, int rows, int cols, int tileSize, int space) {

	SDL_Point targetEdge(0, 0);

	SDL_GetRendererOutputSize(targetRenderer, &targetEdge.x, &targetEdge.y);

	int colsIsOdd = cols % 2;
	int rowsIsOdd = rows % 2;


	int halfRows = float(rows) / 2.0;
	int halfCols = float(cols) / 2.0;

	int tileSizeWithSpace = tileSize + space;

	switch (side) {

		case RIGHT_EDGE:

			for (int i = 1; i <= rows; i++) {

				for (int j = 1; j <= cols; j++) { 

					SDL_Rect tileRect(0,0,tileSize,tileSize);

					tileRect.x = (targetEdge.x - ((2 - j) * tileSizeWithSpace)) - tileSizeWithSpace * (cols - 1);
					tileRect.y = ((targetEdge.y / 2) - ((halfRows + 1 - i) * tileSizeWithSpace)) - (tileSizeWithSpace / 2) * rowsIsOdd;

					SDL_SetRenderDrawColor(targetRenderer, 255, 255, 255, 255);

					SDL_RenderFillRect(targetRenderer, &tileRect);

				}

			}
			break;


		case BOTTOM_EDGE: 

			for (int i = 1; i <= rows; i++) { 
				 
				for (int j = 1; j <= cols; j++) { 

					SDL_Rect tileRect(0, 0, tileSize, tileSize);

					tileRect.x = (((targetEdge.x / 2) - ((halfCols - j) * tileSizeWithSpace) - (tileSizeWithSpace + (0.5 * tileSizeWithSpace * colsIsOdd))) + space * 0.5);
					tileRect.y = (targetEdge.y - ((2 - i) * tileSizeWithSpace)) - tileSizeWithSpace * (rows - 1); // Holy shit getting these to work was a headache but its still mainly based on the magic formula from the other project

					SDL_SetRenderDrawColor(targetRenderer, 255, 255, 255, 255);

					SDL_RenderFillRect(targetRenderer, &tileRect);

				}

			}
			break;


		case LEFT_EDGE:

			for (int i = 1; i <= rows; i++) {

				for (int j = 1; j <= cols; j++) {

					SDL_Rect tileRect(0, 0, tileSize, tileSize);

					tileRect.x = (0 + ((1 - j) * tileSizeWithSpace)) + tileSizeWithSpace * (cols - 1) + space; 
					tileRect.y = ((targetEdge.y / 2) - ((halfRows + 1 - i) * tileSizeWithSpace)) - (tileSizeWithSpace / 2) * rowsIsOdd; 

					SDL_SetRenderDrawColor(targetRenderer, 255, 255, 255, 255); 

					SDL_RenderFillRect(targetRenderer, &tileRect); 
					 
				} 

			}
			break;


		case TOP_EDGE:

			for (int i = 1; i <= rows; i++) {

				for (int j = 1; j <= cols; j++) {

					SDL_Rect tileRect(0, 0, tileSize, tileSize);

					tileRect.x = (((targetEdge.x / 2) - ((halfCols - j) * tileSizeWithSpace) - (tileSizeWithSpace + (0.5 * tileSizeWithSpace * colsIsOdd))) + space * 0.5);
					tileRect.y = (0 + ((1 - i) * tileSizeWithSpace)) + tileSizeWithSpace * (rows - 1) + space;

					SDL_SetRenderDrawColor(targetRenderer, 255, 255, 255, 255);

					SDL_RenderFillRect(targetRenderer, &tileRect);

				}

			}
			break;


	}

}

void RenderInventoryAsGrid(SDL_Renderer* targetRenderer, cint side, cint rows, cint cols, cint tileSize, cint space, inventory& targetInventory, TTF_Font* inventoryFont) {

	SDL_Surface* textSurface = NULL;
	SDL_Texture* itemTexture = NULL;
	SDL_Texture* textTexture = NULL;

	SDL_Point targetEdge(0, 0);

	SDL_GetRendererOutputSize(targetRenderer, &targetEdge.x, &targetEdge.y);

	cint colsIsOdd = cols % 2;
	cint rowsIsOdd = rows % 2;

	cint halfRows = float(rows) / 2.0;
	cint halfCols = float(cols) / 2.0;

	cint tileSizeWithSpace = tileSize + space;

	SDL_SetRenderDrawColor(targetRenderer, 255, 255, 255, 100);

	SDL_Rect tileRect(0, 0, tileSize, tileSize);

	for (int i = 1; i <= rows; i++) {

		for (int j = 1; j <= cols; j++) {

			int currentIndex = ((i - 1) * cols) + (j - 1);
			if (currentIndex < 0) currentIndex = 0;

			switch (side) {

			case RIGHT_EDGE:
				tileRect.x = (targetEdge.x - ((2 - j) * tileSizeWithSpace)) - tileSizeWithSpace * (cols - 1);
				tileRect.y = ((targetEdge.y / 2) - ((halfRows + 1 - i) * tileSizeWithSpace)) - (tileSizeWithSpace / 2) * rowsIsOdd;
				break;

			case BOTTOM_EDGE:

				tileRect.x = (((targetEdge.x / 2) - ((halfCols - j) * tileSizeWithSpace) - (tileSizeWithSpace + (0.5 * tileSizeWithSpace * colsIsOdd))) + space * 0.5);
				tileRect.y = (targetEdge.y - ((2 - i) * tileSizeWithSpace)) - tileSizeWithSpace * (rows - 1); // Holy shit getting these to work was a headache but its still mainly based on the magic formula from the other project
				break;

			case LEFT_EDGE:

				tileRect.x = (0 + ((1 - j) * tileSizeWithSpace)) + tileSizeWithSpace * (cols - 1) + space;
				tileRect.y = ((targetEdge.y / 2) - ((halfRows + 1 - i) * tileSizeWithSpace)) - (tileSizeWithSpace / 2) * rowsIsOdd;
				break;

			case TOP_EDGE:
				tileRect.x = (((targetEdge.x / 2) - ((halfCols - j) * tileSizeWithSpace) - (tileSizeWithSpace + (0.5 * tileSizeWithSpace * colsIsOdd))) + space * 0.5);
				tileRect.y = (0 + ((1 - i) * tileSizeWithSpace)) + tileSizeWithSpace * (rows - 1) + space;
				break;

			}

			SDL_RenderFillRect(targetRenderer, &tileRect);

			if (currentIndex + 1 <= targetInventory.itemArray.size()) {

				SDL_Rect textRect(tileRect);

				int w = 0, h = 0;

				const std::string textString = std::to_string(targetInventory.itemArray[currentIndex].count);
				const char* text = textString.c_str();

				TTF_SizeText(inventoryFont, text, &w, &h);

				textRect.y += tileSizeWithSpace - h;
				textRect.w = w;
				textRect.h = h;

				if (itemTexture != nullptr) {
					SDL_DestroyTexture(itemTexture);
				}

				itemTexture = SDL_CreateTextureFromSurface(targetRenderer, targetInventory.itemArray[currentIndex].itemTextureSurface);

				SDL_RenderCopy(targetRenderer, itemTexture, NULL, &tileRect);

				if (textSurface != nullptr) {
					SDL_FreeSurface(textSurface);
				}

				textSurface = TTF_RenderText_Solid(inventoryFont, text, SDL_Color(255, 0, 0, 255));

				if (textTexture != nullptr) {
					SDL_DestroyTexture(textTexture);
				}

				textTexture = SDL_CreateTextureFromSurface(targetRenderer, textSurface);

				SDL_RenderCopy(targetRenderer, textTexture, NULL, &textRect);

			}

		}

	}

	if (textSurface != nullptr) {
		SDL_FreeSurface(textSurface);
	}

	if (itemTexture != nullptr) {
		SDL_DestroyTexture(itemTexture);
	}

	if (textTexture != nullptr) {
		SDL_DestroyTexture(textTexture);
	}

} //hush

bool CheckCollisions(World world, int focusObjectID) {

	for (int i = 0; i < world.worldObjectArray.size(); i++) {

		if (i != focusObjectID) {

			if (world.worldObjectArray[i].alive) {

				if (world.worldObjectArray[i].inBounds(world.worldObjectArray[focusObjectID].colBox)) {

					return true;

				}

			}

		}

	}
	if (!world.worldObjectArray[focusObjectID].totallyInBounds(world.worldBounds)) {

		return true;

	}
	return false;

} 

Uint32 mineCallback(Uint32 interval, void* param) { //why are you like this sdl

	mineTimerParams* params = static_cast<mineTimerParams*>(param);

	params->objectToMine->damage(params->damageHP);

	if (params->objectToMine->alive) {

		std::cout << "mining, " << params->objectToMine->HP << std::endl;

	}
	else if(params->objectToMine->dropsItems){

		std::cout << "collected " << params->objectToMine->dropsCount << "x " << itemNameMap[params->objectToMine->dropsID] << std::endl;

		params->objectMining->objectInv.insertItemID(params->objectToMine->dropsID, params->objectToMine->dropsCount);

	}
	else {

		std::cout << "mined \n";

	}

	return interval; // Returning the interval means the timer will repeat (dont ask)

}

void DestroyItemSurfaces(inventory& inventory) {

	for (int i = 0; i < inventory.itemArray.size(); i++) {

		SDL_FreeSurface(inventory.itemArray[i].itemTextureSurface);

	}

}

int main() {

	int winHeight = 720, winWidth = 1280;
	int worldHeight = 4000, worldWidth = 4000;

	int worldRes = 40;

	int winHeightCentre = winHeight / 2;
	int winWidthCentre = winWidth / 2;
	int frameStart = 0;
	int frameEnd = 0;
	int playerSpeed = 200;
	int playerMineDamage = 20;
	int playerMineSpeed = 500;
	int startTime = 0;
	int elapsedTime = 0;
	float deltaTime = 0;
	float fps = 0;

	int guyID = 0;
	int rockID = 0;
	int targetedObjectID = 0;

	bool quit = false;
	bool clicking = false;
	bool pressing = false;
	bool inventoryToggle = false;

	SDL_Init(SDL_INIT_EVERYTHING); //hi SDL
	TTF_Init();

#pragma region creating stuff and checks

	SDL_Event windowEvent;

	SDL_Point mousePos;
	SDL_Point worldMousePos;
	SDL_Point camPos(0, 0);
	SDL_Point camSize(winWidth, winHeight);

	TTF_Font* inventoryFont = TTF_OpenFont("Minecraft.ttf", 24);

	SDL_TimerID mineTimerID = 0;

	SDL_Window* window = SDL_CreateWindow("test", WIN_CENTERED, WIN_CENTERED, winWidth, winHeight, 0);

	if (window == nullptr) {
		std::cout << "Epic window fail " << SDL_GetError() << std::endl;
		SDL_Delay(5000);
		return 1;
	}

	SDL_Renderer* windowRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); //window, index, accelerated

	if (windowRenderer == nullptr) {
		std::cout << "Epic renderer fail " << SDL_GetError() << std::endl;
		SDL_Delay(5000);
		return 1;
	}

	SDL_Texture* derrytexture = IMG_LoadTexture(windowRenderer, "image1.bmp");

	World testWorld(windowRenderer, worldWidth, worldHeight);

	WorldObject guy(0, 0, 50, 50, "guy.bmp", 100, true, false);
	WorldObject rock3(100, 400, 300, 300, "rock.png", 500, false, true);

	int seed = SDL_GetPerformanceCounter();

	OpenSimplexNoise noise(seed);

	for (int i = - (worldHeight / 2); i < (worldHeight / 2); i += worldHeight / worldRes) { //gen world

		for (int j = -(worldWidth / 2); j < (worldWidth / 2); j += worldWidth / worldRes) {

			if (noise.eval(j, i, 0) > 0.15 && noise.eval(j, i, 0) < 0.3) {

				std::cout << "putting bush at " << j << ", " << i << "\n";

				WorldObject genBush(j, i, worldWidth / worldRes, worldHeight / worldRes, "bush.png", 100, false, true);
				int genRockID = testWorld.insertWorldObject(genBush);

				testWorld.worldObjectArray[genRockID].dropsID = STICKS_ITEM_ID;
				testWorld.worldObjectArray[genRockID].dropsCount = 10;


			}
			if (noise.eval(j, i, 0) > 0.3 && noise.eval(j, i, 0) < 0.4) {

				std::cout << "putting rock at " << j << ", " << i << "\n";

				WorldObject genRock(j, i, worldWidth / worldRes, worldHeight / worldRes, "rock.png", 100, false, true);
				int genRockID = testWorld.insertWorldObject(genRock);

				testWorld.worldObjectArray[genRockID].dropsID = STONE_ITEM_ID;
				testWorld.worldObjectArray[genRockID].dropsCount = 5;


			}
			
			else {

				std::cout << i << ", " << j << " not putting " << noise.eval(j, i, 1) << "\n";

			}

		}



	}

	int rock3ID = testWorld.insertWorldObject(rock3);

	guyID = testWorld.insertWorldObject(guy);

	testWorld.worldObjectArray[guyID].objectInv.maxSize = 15;

	testWorld.worldObjectArray[rock3ID].dropsID = STONE_ITEM_ID;
	testWorld.worldObjectArray[rock3ID].dropsCount = 40;



#pragma endregion 

	SDL_RenderClear(windowRenderer); //fill render with draw colour

	startTime = SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();

	SDL_SetRenderDrawBlendMode(windowRenderer, SDL_BLENDMODE_BLEND);


	if (inventoryFont == NULL) {

		std::cout << "bad";
		std::cout << TTF_GetError();

	}

	while (!quit) {

		frameStart = SDL_GetPerformanceCounter();

		SDL_RenderCopy(windowRenderer, derrytexture, NULL, NULL);

		SDL_SetRenderDrawColor(windowRenderer, 0, 255, 0, 255);
		SDL_RenderClear(windowRenderer);

#pragma region <------- Input

		while (SDL_PollEvent(&windowEvent)) {
			
			if (windowEvent.motion.type == 1024 || windowEvent.motion.type == 1025 || windowEvent.motion.type == 1026) { // 1024 is moving 1025 is clicking 1026 is letting go

				mousePos.x = windowEvent.motion.x;
				mousePos.y = windowEvent.motion.y;

				//std::cout << mousePos.x << ", " << mousePos.y << "\n";

				//std::cout << windowEvent.motion.type << "\n";

			}

			worldMousePos = SDL_Point(mousePos.x + camPos.x - (winWidth * 0.5) + (camSize.x * 0.5), mousePos.y + camPos.y - (winHeight * 0.5) + (camSize.y * 0.5)); //screen point to world point

			switch (windowEvent.type) {

				case SDL_QUIT: 
					quit = true; 
					break; //if x clicked



				case SDL_KEYDOWN: //for testign only
					if (windowEvent.key.keysym.sym == SDLK_p) {

						testWorld.worldObjectArray[guyID].objectInv.insertItemID(TEST_ITEM_ID, 5);
						testWorld.worldObjectArray[guyID].objectInv.insertItemID(TEST_ITEM_ID_2, 2);

					}
					if (windowEvent.key.keysym.sym == SDLK_e) {

						if (testWorld.worldObjectArray[guyID].objectInv.itemArray.size() > 0) {

							std::cout << "###########" << "\n";

							std::cout << "inventory" << "\n" << "\n";

							for (int i = 0; i < testWorld.worldObjectArray[guyID].objectInv.itemArray.size(); i++) {

								std::cout << testWorld.worldObjectArray[guyID].objectInv.itemArray[i].itemName << " x " << testWorld.worldObjectArray[guyID].objectInv.itemArray[i].count << "\n";

							}

							std::cout << "\n";

							std::cout << "###########" << "\n";

						}

						inventoryToggle = !inventoryToggle;

					}

					break;


				case SDL_MOUSEBUTTONDOWN: //if mouse clicked 
					if (windowEvent.button.button == SDL_BUTTON_RIGHT) {



					}
					else if (windowEvent.button.button == SDL_BUTTON_LEFT) {



					}

					clicking = true;

				case SDL_MOUSEBUTTONUP: //if mouse up (apparently not)

					if (windowEvent.button.state == SDL_RELEASED) { //?????????? If mouse released????? wtf SDL

						clicking = false;
						//std::cout << "mouse up";

					}

					break;

				case SDL_MOUSEWHEEL:
					if (windowEvent.wheel.y > 0 && windowEvent.wheel.y < 20) { //if scroll up also theres some weird bug that causes clicking to set windowevent.wheel.y to somewhere around 600 i have no idea why



					}
					else if (windowEvent.wheel.y < 0) { //if scroll down



					}

					break;

			}



		}

		if (clicking) {

			if (testWorld.objectContainingPoint(worldMousePos) >= 0) { //if pointing at some object

				targetedObjectID = testWorld.objectContainingPoint(worldMousePos);

				if (testWorld.worldObjectArray[targetedObjectID].alive) {

					//std::cout << "clicking" << "\n";

					if (!mineTimerID) {

						mineTimerParams mineParams(playerMineDamage, &testWorld.worldObjectArray[targetedObjectID], &testWorld.worldObjectArray[guyID]);

						mineTimerID = SDL_AddTimer(playerMineSpeed, mineCallback, &mineParams);

					}

				}
				else {

					if (mineTimerID) {

						SDL_RemoveTimer(mineTimerID);
						mineTimerID = 0;

					}

				}

			}
			else {

				if (mineTimerID) {

					SDL_RemoveTimer(mineTimerID);
					mineTimerID = 0;

				}

			}

		}
		else {

			if (mineTimerID) {

				SDL_RemoveTimer(mineTimerID);
				mineTimerID = 0;

			}

		}

		

#pragma region <----- keyboard input

		const Uint8* keyStates = SDL_GetKeyboardState(NULL);

		if (keyStates[SDL_SCANCODE_W]) {

			testWorld.worldObjectArray[guyID].MoveInWorld(0, -1 * playerSpeed * deltaTime);

			if (CheckCollisions(testWorld, guyID)) {

				testWorld.worldObjectArray[guyID].MoveInWorld(0, 1 * playerSpeed * deltaTime);

			}

		}
		if (keyStates[SDL_SCANCODE_A]) {

			testWorld.worldObjectArray[guyID].MoveInWorld(-1 * playerSpeed * deltaTime, 0);

			if (CheckCollisions(testWorld, guyID)) {

				testWorld.worldObjectArray[guyID].MoveInWorld(1 * playerSpeed * deltaTime, 0);

			}

		}
		if (keyStates[SDL_SCANCODE_S]) {

			testWorld.worldObjectArray[guyID].MoveInWorld(0, 1 * playerSpeed * deltaTime);


			if (CheckCollisions(testWorld, guyID)) {

				testWorld.worldObjectArray[guyID].MoveInWorld(0, -1 * playerSpeed * deltaTime);

			}


		}
		if (keyStates[SDL_SCANCODE_D]) {

			testWorld.worldObjectArray[guyID].MoveInWorld(1 * playerSpeed * deltaTime, 0);

			if (CheckCollisions(testWorld, guyID)) {

				testWorld.worldObjectArray[guyID].MoveInWorld(-1 * playerSpeed * deltaTime, 0);

			}

		}
		if (keyStates[SDL_SCANCODE_E]) {



		}
		if (keyStates[SDL_SCANCODE_P]) {

			

		}


#pragma endregion

#pragma endregion

		camPos.x = testWorld.worldObjectArray[guyID].worldPos.x - 0.5 * camSize.x;
		camPos.y = testWorld.worldObjectArray[guyID].worldPos.y - 0.5 * camSize.y;

		RenderWorld(windowRenderer, testWorld, camPos, camSize);

		SDL_RenderDrawPoint(windowRenderer, mousePos.x, mousePos.y);

		if (inventoryToggle) {

			RenderInventoryAsGrid(windowRenderer, RIGHT_EDGE, 5, 3, 50, 5, testWorld.worldObjectArray[guyID].objectInv, inventoryFont);

		}

		SDL_RenderPresent(windowRenderer); //show render

		//std::cout << SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency() - startTime << "\n";

		//std::cout << testWorld.worldObjectArray[guyID].worldPos.x << "," << testWorld.worldObjectArray[guyID].worldPos.y << "\n";

		//std::cout << fps << "\n";

		//cap fps

		SDL_Delay(4);

		//DELTATIME
		
		frameEnd = SDL_GetPerformanceCounter();

		elapsedTime = frameEnd - frameStart;

		deltaTime = float(elapsedTime) / SDL_GetPerformanceFrequency();

		fps = (1.0f / deltaTime);

	}

	DestroyItemSurfaces(testWorld.worldObjectArray[guyID].objectInv);

	TTF_CloseFont(inventoryFont);

	SDL_DestroyTexture(derrytexture);

	SDL_DestroyRenderer(windowRenderer);  //destroy the stuff we dont need it anymore
	SDL_DestroyWindow(window);

	TTF_Quit();
	SDL_Quit(); //bye SDL
	return 0;

} 

//what is up guys
#include <game.h>
#include <common.h>
#include <sfx.h>
#include <g3dhax.h>
#include <stage.h>
#include "boss.h"

const char* AKarcNameList[] = {
	"kameck",
	"iron_ball",
	"killer",
	"nokonokoA",
	"nokonokoB",
	"wing",
	NULL
};

class daNewKameck : public dEn_c {
public:

	mHeapAllocator_c allocator;
	m3d::mdl_c bodyModel;
	m3d::anmChr_c animationChr;
	nw4r::g3d::ResFile resFile;
	ActivePhysics::Info HitMeBaby;

	void updateModelMatrices();
	void bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate);

	int onCreate();
	int onExecute();
	int onDelete();
	int onDraw();

	int health = 25;
	int fireballHits = 0;
	u8 event;
	int phase = 1;
	daNewKameck *Kamek;
	int direction;
	int timer;
	int timerAlt = 0;
	bool left;
	int ly = 0;
	float initY, initX;
	f32 speddTwo = 1.0;
	bool WasJustDamaged;
	u32 cmgr_returnValue;
	bool isBouncing;

	static daNewKameck* build();

	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	void yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	bool collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther); 
	bool collisionCat5_Mario(ActivePhysics *apThis, ActivePhysics *apOther); 
	bool collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther); 
	bool collisionCat8_FencePunch(ActivePhysics *apThis, ActivePhysics *apOther); 
	bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther); 
	bool collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther); 
	bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther); 
	bool collisionCat11_PipeCannon(ActivePhysics *apThis, ActivePhysics *apOther); 
	bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther); 
	bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther); 
	bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther); 
	bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther); 
	bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther);

	bool MakeKamekFlyDammit(bool editRot,double a,int b,f32 DaSpeed);
	bool calculateTileCollisions();

	USING_STATES(daNewKameck);
	DECLARE_STATE(Start);
	DECLARE_STATE(Teleport);
	DECLARE_STATE(Attack);
	DECLARE_STATE(Shield);
	DECLARE_STATE(End);
	DECLARE_STATE(Fly);
};

CREATE_STATE(daNewKameck, Start);
CREATE_STATE(daNewKameck, Teleport);
CREATE_STATE(daNewKameck, Attack);
CREATE_STATE(daNewKameck, Shield);
CREATE_STATE(daNewKameck, End);
CREATE_STATE(daNewKameck, Fly);

bool daNewKameck::MakeKamekFlyDammit(bool editRot,double a,int b,f32 DaSpeed) {
    if (editRot) {
    	if (b == 0) {
    		this->rot.x += a;
    	} else if (b == 1) {
    		this->rot.y += a;
    	} else if (b == 2) {
    		this->rot.z += a;
    	} else { return false; }
    }

    switch (b) {
    	case 0:
    	  this->pos.x += DaSpeed;
    	  break;
    	case 1:
    	  this->pos.y += DaSpeed;
    	  break;
    	default:
    	  return false;
    }

	if ((this->pos.y <= (initY - 100)) || (this->pos.y >= (initY + 100))) {
		pos.y -= (this->pos.y >= (initY + 100)) ? 1.0 : -1.0;
		speddTwo *= -1;
	}

	return true;
}

daNewKameck* daNewKameck::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daNewKameck));
	return new(buffer) daNewKameck;
}

void daNewKameck::updateModelMatrices() {
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}

void daNewKameck::bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate) {
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr(name);
	this->animationChr.bind(&this->bodyModel, anmChr, unk);
	this->bodyModel.bindAnim(&this->animationChr, unk2);
	this->animationChr.setUpdateRate(rate);
}
int daNewKameck::onCreate() {

	allocator.link(-1, GameHeaps[0], 0, 0x20);

	resFile.data = getResource("kameck", "g3d/kameck.brres");
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("kameck");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_Boss(&bodyModel, 0);
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr("wait");
	this->animationChr.setup(mdl, anmChr, &this->allocator, 0);

	allocator.unlink();

	HitMeBaby.xDistToCenter = 0.0; 
	HitMeBaby.yDistToCenter = 15.0; 
	HitMeBaby.xDistToEdge = 12.0; 
	HitMeBaby.yDistToEdge = 16.0; 
	HitMeBaby.category1 = 0x3; 
	HitMeBaby.category2 = 0x0; 
	HitMeBaby.bitfield1 = 0x4F; 
	HitMeBaby.bitfield2 = 0xFFFE7FFF; 
	HitMeBaby.unkShort1C = 0; 
	HitMeBaby.callback = &dEn_c::collisionCallback; 
	this->aPhysics.initWithStruct(this, &HitMeBaby); 
	this->aPhysics.addToList(); 

	this->scale.x = 1.0; 
	this->scale.y = 1.0; 
	this->scale.z = 1.0;

	this->initY = this->pos.y;
	this->initX = this->pos.x;

	static const lineSensor_s below(12<<12, 4<<12, 0<<12);
	static const pointSensor_s above(0<<12, 12<<12);
	static const lineSensor_s adjacent(6<<12, 9<<12, 14<<12);

	collMgr.init(this, &below, &above, &adjacent);
	collMgr.calculateBelowCollisionWithSmokeEffect();

	cmgr_returnValue = collMgr.isOnTopOfTile();

	if (collMgr.isOnTopOfTile())
    {   isBouncing = false; }
	else
	{	isBouncing = true;  }

	this->rot.y = (direction) ? 0xD800 : 0x2800;

	event = ((this->settings & 0x000F0000) >> 16) - 1;

	bindAnimChr_and_setUpdateRate("wait", 1, 0.0, 1.0);

	OSReport("It's time, Mario! Come on up and face me!\n");

	doStateChange(&StateID_Start);

	this->onExecute();

	return true;
}

int daNewKameck::onExecute() {
	acState.execute();

	updateModelMatrices();
	bodyModel._vf1C();
	
	S16Vec nullRot = {0,0,0};
	Vec oneVec = {1.0f, 1.0f, 1.0f};

	dStateBase_c *s = acState.getCurrentState();

	if ((this->animationChr.isAnimationDone() && s != &StateID_End) &&
	(s != &StateID_Teleport)) {
		this->animationChr.setCurrentFrame(0.0);
	}

	u8 facing = dSprite_c__getXDirectionOfFurthestPlayerRelativeToVEC3(this, this->pos);
	if (s != &StateID_End) {
		if (facing != this->direction) {
			this->direction = facing;
		}
	}

	rot.y = (s != &StateID_End && s != &StateID_Shield) ? (direction) ? 0xD800 : 0x2800 : 0x0000;

	if ((s != &StateID_Shield && s != &StateID_Teleport) && s != &StateID_Fly) {
		bool ret = calculateTileCollisions();
		if (ret) {
			pos.x = (f32)initX;
			pos.y = (f32)initY;
			this->left = false;
			doStateChange(&StateID_Teleport);
		}
	}

	dStageActor_c *enemy;
    if (this->timerAlt == 100  && s == &StateID_Fly) {
		// Please work...
		SpawnEffect("Wm_en_landsmoke", 0, &pos, &nullRot, &oneVec);
		enemy = CreateActor(55, 0x12, (Vec){pos.x, (f32)(pos.y - 5.0), pos.z}, 0, 0);

		enemy->scale = (Vec){1.0, 1.0, 1.0};
		enemy->speed.x = (direction) ? 1.0 : -1.0;
		enemy->speed.y = 2.0;
	}

	if (this->timerAlt > 100) {
		this->timerAlt = 0;
	}

	this->timerAlt++;

	return true;
}

int daNewKameck::onDelete() {
	return true;
}

int daNewKameck::onDraw() {
    bodyModel.scheduleForDrawing();

	return true;
}

// States //
void daNewKameck::beginState_Start() {
	this->timer = 0;
}
void daNewKameck::executeState_Start() {
	if (this->timer >= 100) {
		doStateChange(&StateID_Attack);
	}
	this->timer++;
}
void daNewKameck::endState_Start() {}

void daNewKameck::beginState_Teleport() {
	this->timer = 0;
	S16Vec nullRot = {0,0,0};
	Vec oneVec = {1.0f, 1.0f, 1.0f};

	this->aPhysics.removeFromList();
	
	SpawnEffect("Wm_mr_cmnsndlandsmk", 0, &pos, &nullRot, &oneVec);
	SpawnEffect("Wm_en_landsmoke", 0, &pos, &nullRot, &oneVec);
	SpawnEffect("Wm_en_sndlandsmk_s", 0, &pos, &nullRot, &oneVec);

	if (this->WasJustDamaged) {
		this->WasJustDamaged = (!this->ly);
	}

	pos.x = (this->left) ? pos.x - 200 : pos.x + 200;
	if (left) {
		if (pos.x < (initX)) {
			pos.x += ((f32)initX - pos.x);
		}
	} else {
        if (pos.x > (initX + 200)) {
			pos.x -= 200;
		}
	}
	pos.y = (f32)initY;

	bindAnimChr_and_setUpdateRate("appear", 1, 0.0, 1.0);
	PlaySound(this, SE_BOSS_KAMECK_APP);
}
void daNewKameck::executeState_Teleport() {
    if (animationChr.isAnimationDone()) {
		dStateBase_c *t;
		t = (this->ly == 0) ? &StateID_Fly : &StateID_Attack;
		this->ly++;
		if (this->ly > 2) { this->ly = 0; }
	    doStateChange(t);
	}
}
void daNewKameck::endState_Teleport() {
	HitMeBaby.xDistToEdge = 12.0; 
	HitMeBaby.yDistToEdge = 16.0;
	this->aPhysics.initWithStruct(this, &HitMeBaby); 
	this->aPhysics.addToList(); 
	this->left = !this->left;
	OSReport("Now I will change my direction from %s to %s.\n", (left) ? "left" : "right", (left) ? "right" : "left");
}

void daNewKameck::beginState_Attack() {
	this->timer = 0;
	bindAnimChr_and_setUpdateRate("wait", 1, 0.0, 1.0);
}
void daNewKameck::executeState_Attack() {

	this->timer++;
	if (timer == 260) {
		doStateChange(&StateID_Teleport);
	}
	else if (timer == 125) {
		bindAnimChr_and_setUpdateRate("throw_ed", 1, 0.0, 1.0);
	}
	else if (timer == 84) {
		bindAnimChr_and_setUpdateRate("throw_st_wait", 1, 0.0, 1.0);
	}	
	else if (timer == 154) {
		bindAnimChr_and_setUpdateRate("throw_ed_wait", 1, 0.0, 1.0);
		// Spawning an actor ("Spike Ball" or "Bull's-Eye Bill")
		dStageActor_c *spawn;
		dStageActor_c *newSpawn;
		int a = ((this->health % 2) == 0) ? 128 : 89;
		int b = (a == 128) ? (direction) ? 0x12 : 0x2 : (direction) ? 0x2 : 0x12;

		OSReport("Here's a little \"%s\" for you!\n", (a == 128) ? "Bull's-Eye Bill" : "Spike Ball"); 

		spawn = CreateActor(a, b, (Vec){pos.x, (pos.y + (f32)8.6), pos.z}, 0, 0);

		spawn->scale.x = (a == 89) ? 0.9 : 1.3;
		spawn->scale.y = (a == 89) ? 0.9 : 1.3;
		spawn->scale.z = (a == 89) ? 0.9 : 1.3;

		spawn->speed.x = (direction) ? -2.0 : 2.0;
		spawn->speed.y = 2.0;

		if (phase > 2 && a == 128) {
			newSpawn = CreateActor(a, b, (Vec){pos.x, (pos.y + (f32)7.6), pos.z}, 0, 0);

		    newSpawn->scale.x = 1.3;
		    newSpawn->scale.y = 1.3;
		    newSpawn->scale.z = 1.3;

		    newSpawn->speed.x = (direction) ? -3.0 : 3.0;
		    newSpawn->speed.y = 3.0;
		}
	}
	else if (timer == 170) {
		bindAnimChr_and_setUpdateRate("wait", 1, 0.0, 1.0);
	}
	else if (timer == 220) {
		bindAnimChr_and_setUpdateRate("disappear", 1, 0.0, 1.0);
		this->aPhysics.removeFromList();
	}

}
void daNewKameck::endState_Attack() {}

void daNewKameck::beginState_Shield() {
	if (this->fireballHits) { this->fireballHits = 0; }
	this->timer = 0;
	bindAnimChr_and_setUpdateRate("damage_s", 1, 0.0, 1.0);
	this->aPhysics.removeFromList();
	PlaySound(this, SE_BOSS_KAMECK_DOWN);
}
void daNewKameck::executeState_Shield() {
	if (this->health <= 0) {
		doStateChange(&StateID_End);
	}

	this->timer++;
	if (this->timer == 120) {
		bindAnimChr_and_setUpdateRate("disappear", 1, 0.0, 1.0);
		PlaySound(this, SE_BOSS_KAMECK_DISAPP);
	}
	else if (this->timer == 160) {
		doStateChange(&StateID_Teleport);
	}
}
void daNewKameck::endState_Shield() {
	if (this->health > 0)
	{ OSReport("My current phase: %d\n", this->phase); }
	this->WasJustDamaged = true;
}

void daNewKameck::beginState_End() {
	this->timer = 0;
	this->removeMyActivePhysics();
	PlaySound(this, SE_BOSS_CMN_DAMAGE_LAST);
	bindAnimChr_and_setUpdateRate("demo_escapeA1", 1, 0.0, 1.0);
}
void daNewKameck::executeState_End() {
	if (this->timer == 185) { bindAnimChr_and_setUpdateRate("disappear_st", 1, 0.0, 1.0); }

	else if (this->timer >= 194) {
		if (timer == 194) {
			bindAnimChr_and_setUpdateRate("disappear", 1, 0.0, 1.0);
			PlaySound(this, SE_BOSS_KAMECK_DISAPP);
		}

        if (this->animationChr.isAnimationDone()) {
			bool eventActive = dFlagMgr_c::instance->active(event);
	        if (!eventActive) {
	    	    dFlagMgr_c::instance->set(event, 0, true, false, false);
	        }
		    OSReport("Goodbye, Mario! Hope to see you again! Bhahahahahahahaha!\n");
		    this->Delete(1);
		}
	}
	this->timer++;
}
void daNewKameck::endState_End() {}

void daNewKameck::beginState_Fly() {
	this->timer = 0;
	S16Vec nullRot = {0,0,0};
	Vec oneVec = {1.0f, 1.0f, 1.0f};

	SpawnEffect("Wm_mr_cmnsndlandsmk", 0, &pos, &nullRot, &oneVec);
	SpawnEffect("Wm_en_landsmoke", 0, &pos, &nullRot, &oneVec);
	SpawnEffect("Wm_en_sndlandsmk_s", 0, &pos, &nullRot, &oneVec);

	this->pos.y = (direction) ? pos.y : pos.y - 20;

	bindAnimChr_and_setUpdateRate("fly", 1, 0.0, 0.6);

	HitMeBaby.xDistToEdge = 16.0; 
	HitMeBaby.yDistToEdge = 12.0;

	this->aPhysics.initWithStruct(this, &HitMeBaby); 
	this->aPhysics.addToList(); 
}
void daNewKameck::executeState_Fly() {
	// All of this code makes the boss fly around
	
	PlaySound(this, SE_AMB_WIND);
	f32 spedd = (direction) ? (f32)-1.0 : (f32)1.0;
	if (this->timer <= 320) { 
		// Look at the function "MakeKamekFlyDammit" to understand the values
		MakeKamekFlyDammit(false, 0.0, 0, spedd);
		MakeKamekFlyDammit(false, 0.0, 1, this->speddTwo);
	}
	else {
		if (this->timer == 322) {
		    bindAnimChr_and_setUpdateRate("disappear", 1, 0.0, 1.0);
		} else if (this->timer >= 302) {
			S16Vec nullRot = {0,0,0};
	        Vec oneVec = {1.0f, 1.0f, 1.0f};
	        SpawnEffect("Wm_mr_cmnsndlandsmk", 0, &pos, &nullRot, &oneVec);
	        SpawnEffect("Wm_en_landsmoke", 0, &pos, &nullRot, &oneVec);
	        SpawnEffect("Wm_en_sndlandsmk_s", 0, &pos, &nullRot, &oneVec);
		    pos.x = (f32)initX;
		    pos.y = (f32)initY;
			this->WasJustDamaged = false;
		    doStateChange(&StateID_Teleport);
		}
	}
	this->timer++;
}
void daNewKameck::endState_Fly() {
    this->WasJustDamaged = false;
}

// Collisions //
void daNewKameck::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
	char th = usedForDeterminingStatePress_or_playerCollision(this, apThis, apOther, 0);
	if (!this->WasJustDamaged) {
	    if (th == 1 || th == 3) {
		    this->phase++;
		    this->health -= 5;
			
		    doStateChange(&StateID_Shield);
			
			this->WasJustDamaged = true;
	    } else if (th == 0) {
		    DamagePlayer(this, apThis, apOther);
	    }
		return;
	}
	DamagePlayer(this, apThis, apOther);
	return;
}
void daNewKameck::yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
	this->playerCollision(apThis, apOther);
}

bool daNewKameck::collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther) {
	if (acState.getCurrentState()== &StateID_Fly || WasJustDamaged) { return false; }

	this->WasJustDamaged = true;
	this->health -= 5;
	this->phase++;

	doStateChange(&StateID_Shield);

	return false;
} 
bool daNewKameck::collisionCat5_Mario(ActivePhysics *apThis, ActivePhysics *apOther) {
	return this->collisionCat3_StarPower(apThis, apOther);
} 
bool daNewKameck::collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther) {
	return this->collisionCat3_StarPower(apThis, apOther);
} 
bool daNewKameck::collisionCat8_FencePunch(ActivePhysics *apThis, ActivePhysics *apOther) {
	return false;
} 
bool daNewKameck::collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther) {
	return this->collisionCat3_StarPower(apThis, apOther);
}
bool daNewKameck::collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther) {
	return this->collisionCat3_StarPower(apThis, apOther);
} 
bool daNewKameck::collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther) {
	DamagePlayer(this, apThis, apOther);
	return false;
} 
bool daNewKameck::collisionCat11_PipeCannon(ActivePhysics *apThis, ActivePhysics *apOther) {
	return this->collisionCat3_StarPower(apThis, apOther);
} 
bool daNewKameck::collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther) {
	return false;
} 
bool daNewKameck::collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther) {
	if (acState.getCurrentState()== &StateID_Fly || WasJustDamaged) { return false; }
	this->fireballHits++;
    if (this->fireballHits >= 5) {
    	PlaySound(this, SE_EMY_DOWN);

		this->WasJustDamaged = true; 
    	this->phase++;
    	this->fireballHits = 0;
		this->health -= 5;

    	doStateChange(&StateID_Shield);
    }
    return false;
} 
bool daNewKameck::collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther) {
	return false;
}
bool daNewKameck::collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther) {
	return this->collisionCat1_Fireball_E_Explosion(apThis, apOther);
}
bool daNewKameck::collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther) {
	return this->collisionCat1_Fireball_E_Explosion(apThis, apOther);
}

bool daNewKameck::calculateTileCollisions() {
	// Returns true if sprite should turn, false if not.

	HandleXSpeed();
	HandleYSpeed();
	doSpriteMovement();

	cmgr_returnValue = collMgr.isOnTopOfTile();
	collMgr.calculateBelowCollisionWithSmokeEffect();

	if (isBouncing) {
		stuffRelatingToCollisions(0.1875f, 1.0f, 0.5f);
		if (speed.y != 0.0f)
		{	isBouncing = false;   }
	}

	if (collMgr.isOnTopOfTile()) {
		// Walking into a tile branch

		if (cmgr_returnValue == 0)
		{	isBouncing = true;   }

		speed.y = 0.0f;

		max_speed.x = (direction == 1) ? -0.8f : 0.8f;
	} else {
		x_speed_inc = 0.0f;
	}

	// Bouncing checks
	if (_34A & 4) {
		Vec v = (Vec){0.0f, 1.0f, 0.0f};
		collMgr.pSpeed = &v;

		if (collMgr.calculateAboveCollision(collMgr.outputMaybe))
		{	speed.y = 0.0f;   }

		collMgr.pSpeed = &speed;

	} else {
		if (collMgr.calculateAboveCollision(collMgr.outputMaybe))
		{	speed.y = 0.0f;   }
	}

	collMgr.calculateAdjacentCollision(0);

	// Switch Direction
	if (collMgr.outputMaybe & (0x15 << direction)) {
		if (collMgr.isOnTopOfTile()) {
			isBouncing = true;
		}
		return true;
	}
	return false;
}

#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"
#include "bg_vehicleinfo.h"
#include "bg_weaponinfo.h"
#include "bg_datamanager.h"


// decls
int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
void	trap_FS_FCloseFile( fileHandle_t f );




const std::string GameObjectInfo::baseModelPath = "models/vehicles/";

GameObjectInfo::GameObjectInfo() :
	descriptiveName_(""),
	tinyName_(""),
	modelName_(""),
	modelPath_(""),
	gameSet_(0),
	category_(0),
	class_(0),
	flags_(0),
	caps_(0),
	renderFlags_(0),
	maxHealth_(0),
	shadowShader_(SHADOW_NONE),
	maxSpeed_(0)
{
	VectorSet( turnSpeed_, 0, 0, 0 );
	VectorSet( camDist_, 0, 0, 0 );
	VectorSet( camHeight_, 0, 0, 0 );
	Vector4Set( shadowCoords_, 0, 0, 0, 0 );
	Vector4Set( shadowAdjusts_, 0, 0, 0, 0 );
	VectorSet( mins_, 0, 0, 0 );
	VectorSet( maxs_, 0, 0, 0 );
}

GameObjectInfo::~GameObjectInfo()
{
}

GameObjectInfo*
GameObjectInfo::createVehicle(unsigned int category)
{
	GameObjectInfo* newVehicle = 0;

	switch( category )
	{
	case GameObjectInfo::GO_CAT_PLANE:
		newVehicle = new GameObjectInfo_Plane();	
		break;
	case GameObjectInfo::GO_CAT_HELO:
		newVehicle = new GameObjectInfo_Helicopter();
		break;
	case GameObjectInfo::GO_CAT_GROUND:
		newVehicle = new GameObjectInfo_GroundVehicle();
		break;
	case GameObjectInfo::GO_CAT_BOAT:
		newVehicle = new GameObjectInfo_Boat();
		break;
	case GameObjectInfo::GO_CAT_INF:
		newVehicle = new GameObjectInfo_Infantry();
		break;
	default:
		Com_Error(ERR_FATAL, "Invalid vehicle class in createVehicle!");
		return 0;
	}

	if( !newVehicle )
	{
		Com_Error(ERR_FATAL, "Unable to allocate memory for GameObjectInfo!");
		return 0;
	}
	return newVehicle;
}

bool
GameObjectInfo::setupGameObject()
{
	// always add an empty loadout for customization
	defaultLoadouts_.insert(std::make_pair("Custom", Loadout()));

	if( !createModelPath() )
		return false;

	if( !setupBoundingBox() )
		return false;

	return true;
}

bool
GameObjectInfo::createModelPath()
{
	modelPath_ = baseModelPath;

	switch( category_ )
	{
	case GO_CAT_PLANE:
		modelPath_ += "planes/";
		break;
	case GO_CAT_HELO:
		modelPath_ += "helos/";
		break;
	case GO_CAT_GROUND:
		modelPath_ += "ground/";
		break;
	case GO_CAT_BOAT:
		modelPath_ += "sea/";
		break;
	case GO_CAT_INF:
		modelPath_ += "lqms/";
		break;
	default:
		Com_Error(ERR_FATAL, "Wrong category in vehicle in 'getModelPath'");
		return false;
	}
	modelPath_ += modelName_ + "/" + modelName_;
	return true;
}

std::string
GameObjectInfo::getModelPath( bool extension )
{
	if( !modelPath_.length() || modelPath_ == "" )
		createModelPath();

	if( extension )
		return (modelPath_ + ".md3");

	return modelPath_;
}

int
GameObjectInfo::getTagsContaining( std::string const& filename, 
								std::string const& str,
								std::vector<md3Tag_t>& tagList )
{
	if( str.empty() || str == "" ) 
		return 0;

	tagList.clear();
	fileHandle_t	f;
	if( trap_FS_FOpenFile(filename.c_str(), &f, FS_READ) >= 0 ) 
	{
		md3Header_t head;
		md3Frame_t	frame;
		md3Tag_t	tag;
		trap_FS_Read(&head, sizeof(head), f);
		for( int i = 0; i < head.numFrames; ++i ) 
			trap_FS_Read(&frame, sizeof(frame), f);
		int total = head.numTags;
		for( int i = 0; i < total; ++ i ) 
		{
			trap_FS_Read(&tag, sizeof(tag), f);
			std::string tagName(tag.name);
			// if it contains the string at the first position, then add it
			if( tagName.find(str) == 0 )
				tagList.push_back(tag);
		}
		trap_FS_FCloseFile(f);
	} 
	else 
	{
		Com_Printf( "Unable to open file %s\n", filename.c_str() );
	}
	return tagList.size();
}

bool
GameObjectInfo::addWeaponToLoadout( Loadout& loadout, 
									std::string const& lookupName,
									std::string const& displayName,
									int maxAmmo,
									unsigned int selectionType,
									int turret,
									bool limitedAngles,
									vec3_t minAngles,
									vec3_t maxAngles )
{
	int idx = DataManager::getInstance().findWeaponByName(lookupName);
	if( idx < 0 )
	{
		Com_Printf("Unable to add weapon with name '%s' to loadout, didnt find it.", lookupName.c_str());
		return false;
	}
	Armament arm;
	arm.displayName_ = displayName;
	arm.maxAmmo_ = maxAmmo;
	arm.weaponIndex_ = idx;
	arm.selectionType_ = selectionType;
	if( limitedAngles && minAngles && maxAngles )
	{
		arm.limitedAngles_ = limitedAngles;
		VectorCopy( minAngles, arm.minAngles_ );
		VectorCopy( maxAngles, arm.maxAngles_ );
	}
	loadout.push_back(arm);

	return true;
}

					

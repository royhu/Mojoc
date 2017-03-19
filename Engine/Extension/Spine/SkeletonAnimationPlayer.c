/*
 *
 *
 *  Created on: 2013-7-28
 *      Author: scott.cgi
 */

#include <math.h>

#include "Engine/Extension/Spine/SkeletonAnimationPlayer.h"
#include "Engine/Toolkit/Platform/Log.h"
#include "Engine/Graphics/OpenGL/GLPrimitive.h"


static inline void UpdateNormal(SkeletonAnimationPlayer* player, float deltaTime)
{
	if (player->curTime < player->curAnimationData->duration)
	{
		ASkeleton->Apply(player->skeleton, player->curAnimationData, player->curTime, 1.0f);
		ASkeletonDraw   (player->skeleton);

		player->curTime += deltaTime;

	}
	else
	{
		ASkeleton->Apply(player->skeleton, player->curAnimationData, player->curAnimationData->duration, 1.0f);
		ASkeletonDraw   (player->skeleton);

		if (player->OnActionOver != NULL)
		{
			// action over function may call SetAnimationMix
			// so we need keep cuTime value
			player->OnActionOver(player);
		}

		if (player->loop > 0)
		{
			player->loop--;
		}

		player->curTime = 0.0f;
	}
}


static inline void UpdateMix(SkeletonAnimationPlayer* player, float deltaTime)
{
	if (player->preTime < player->preAnimationData->duration)
	{
        ASkeleton->Apply(player->skeleton, player->preAnimationData, player->preTime, 1.0f);
        player->preTime += deltaTime;
	}
    else
    {
        ASkeleton->Apply(player->skeleton, player->preAnimationData, player->preAnimationData->duration, 1.0f);
    }


	float mixPercent = player->mixTime / player->mixDuration;
	if (mixPercent < 1.0f)
	{
        player->mixTime += deltaTime;
	}
    else
    {
        mixPercent               = 1.0f;
        player->preAnimationData = NULL;
    }


    if (player->curTime < player->curAnimationData->duration)
    {
        ASkeleton->Apply(player->skeleton, player->curAnimationData, player->curTime, mixPercent);
        ASkeletonDraw   (player->skeleton);

        player->curTime += deltaTime;
    }
    else
    {
        ASkeleton->Apply(player->skeleton, player->curAnimationData, player->curAnimationData->duration, mixPercent);
        ASkeletonDraw   (player->skeleton);

        if (player->OnActionOver != NULL)
        {
            // action over function may call SetAnimationMix
            // so we need keep cuTime value
            player->OnActionOver(player);
        }

        player->curTime = 0.0f;
    }
}


static void Update(SkeletonAnimationPlayer* player, float deltaTime)
{
    if (player->loop != 0)
    {
        if (player->preAnimationData == NULL)
        {
            UpdateNormal(player, deltaTime);
        }
        else
        {
            UpdateMix(player, deltaTime);
        }
    }
}


static inline void InitSkeletonAnimationPlayer(SkeletonAnimationPlayer* player, SkeletonData* skeletonData, const char* animationName)
{
	ASkeleton->Init(skeletonData, player->skeleton);
	player->preAnimationData      = NULL;
	player->curAnimationData      = ASkeletonGetAnimationData(player->skeleton, animationName);
	ALogA(player->curAnimationData, "SkeletonAnimationPlayer can not find animtionData by name = %s", animationName);

	player->curTime               = 0.0f;
	player->preTime               = 0.0f;
	player->mixTime               = 0.0f;
	player->mixDuration           = 0.0f;
	player->OnActionOver          = NULL;
	player->loop                  = -1;

	//ASkeletonAnimationPlayer->Release(player);
	//ALogA(0, "stop");
}



static void Release(SkeletonAnimationPlayer* player)
{
	ASkeleton->Release(player->skeleton);
}


static void SetAnimation(SkeletonAnimationPlayer* player, const char* animationName)
{
	player->curAnimationData = ASkeletonGetAnimationData(player->skeleton,  animationName);

	ALogA(player->curAnimationData != NULL, "SetAnimation can not find animtionData by name = %s", animationName);
	player->curTime = 0.0f;
}

static void SetAnimationMix(SkeletonAnimationPlayer* player, const char* animationName, float mixDuration)
{
	player->preAnimationData = player->curAnimationData;
	player->preTime          = player->curTime;
	player->curTime          = 0.0f;
	player->mixTime          = 0.0f;
	player->mixDuration      = mixDuration;
	player->curAnimationData = ASkeletonGetAnimationData(player->skeleton,  animationName);
	ALogA(player->curAnimationData != NULL, "SetAnimationMix can not find animtionData by name = %s", animationName);
}


static SkeletonAnimationPlayer* CreateWithData(SkeletonData* skeletonData, const char* animationName)
{
	SkeletonAnimationPlayer* player = (SkeletonAnimationPlayer*) malloc(sizeof(SkeletonAnimationPlayer));
	InitSkeletonAnimationPlayer(player, skeletonData, animationName);

	return player;
}

static SkeletonAnimationPlayer* Create(const char* filePath, const char* animationName)
{
	SkeletonAnimationPlayer* player = (SkeletonAnimationPlayer*) malloc(sizeof(SkeletonAnimationPlayer));
	InitSkeletonAnimationPlayer(player, ASkeletonData->Get(filePath), animationName);

	return player;
}

static void Init(const char* filePath, const char* animationName, SkeletonAnimationPlayer* outPlayer)
{
	InitSkeletonAnimationPlayer(outPlayer, ASkeletonData->Get(filePath), animationName);
}

static void Render(Drawable* drawable)
{
	SkeletonSlot* slot        = (SkeletonSlot*) drawable->userData;
	*AGLPrimitive->color      = *slot->color;
	AGLPrimitive->modelMatrix = slot->bone->drawable->modelMatrix;

	AGLPrimitive->DrawPolygon(ASkeletonSlotGetBoundingBox(slot)->vertexArr);
}

static void InitSlotBoundingBoxDrawable(SkeletonAnimationPlayer* player, const char* slotName, Drawable* outDrawable)
{
	SkeletonSlot* slot = ASkeletonAnimationPlayerGetSlot(player, slotName);
	ALogA(slot != NULL, "InitSlotBoundingBoxDrawable not found SkeletonSlot by slotName = %s", slotName);

	ADrawable->Init(outDrawable);

	outDrawable->userData = slot;
	outDrawable->Render   = Render;
}

struct ASkeletonAnimationPlayer ASkeletonAnimationPlayer[1] =
{
	Create,
	CreateWithData,
	Init,

	Release,
	Update,

	SetAnimation,
	SetAnimationMix,

	InitSlotBoundingBoxDrawable,
};
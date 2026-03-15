#include <jni.h>
#include <string>
#include <thread>
#include <vector>

#include "logger.h"
#include <BNM/Image.hpp>
#include <BNM/Class.hpp>
#include <BNM/Field.hpp>
#include <BNM/Method.hpp>
#include <BNM/Loading.hpp>
#include <BNM/Property.hpp>
#include "utils.h"


void OnLoaded();

extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    BNM::Loading::AddOnLoadedEvent(OnLoaded);
    BNM::Loading::TryLoadByJNI(env);
    return JNI_VERSION_1_6;
}

// FeatureTypes: Toggle, Seekbar, Category
// Examples:
// Toggle:ToggleName:true
// Seekbar:SeekbarName:1_20_10
// Category:CategoryName
extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_android_support_Menu_getFeatureList(JNIEnv *env, jobject thiz) {
    std::string feats[] = {
            "Toggle:Characters",
            "Seekbar:Reward:1_20",
    };
    return toJobjectArray(env, feats);
}

struct Feature {
    bool characters{false};
    int reward{1};
};

Feature feature{};

extern "C" JNIEXPORT void JNICALL
Java_com_android_support_Menu_valueChange(
        JNIEnv *env,
        jobject thiz,
        jint featIdx,
        jstring featName,
        jint value
) {
    // featIdx: index in feature list
    switch (featIdx) {
        case 0: {
            feature.characters = value;
            break;
        }
        case 1: {
            feature.reward = value;
            break;
        }
        default:
            break;
    }
}

void (*old_AddItem)(void *instance, void *item, int count);

void new_AddItem(void *instance, void *item, int count) {
    return old_AddItem(instance, item, count * feature.reward);
}

void (*old_AddTemptation)(void *instance, int added);

void new_AddTemptation(void *instance, int added) {
    return old_AddTemptation(instance, added * feature.reward);
}

void (*old_UserCharacterCtor)(void *instance, void *collectedUserCharacter, void *characterStatic);

void new_UserCharacterCtor(BNM::IL2CPP::Il2CppObject *instance, void *collectedUserCharacter,
                           void *characterStatic) {
    old_UserCharacterCtor(instance, collectedUserCharacter, characterStatic);
    if (feature.characters) {
        GetMethod<void>(instance, "AddExperience")(100);
    }
}

int (*old_GetLocationCurrentLevel)(void *instance, void *config);

int new_GetLocationCurrentLevel(BNM::IL2CPP::Il2CppObject *instance,
                                BNM::IL2CPP::Il2CppObject *config) {
    int level = old_GetLocationCurrentLevel(instance, config);
    if (level > 5) {
        auto staticItems = GetProperty<BNM::IL2CPP::Il2CppObject *>(config, "LocationKey");
        auto userInventoryUsecase = GetField<BNM::IL2CPP::Il2CppObject *>(instance,
                                                                          "userInventoryUsecase");
        auto RemoveItem = GetMethod<void>(userInventoryUsecase, "RemoveItem");
        RemoveItem(staticItems, level - 5);
        level = 5;
    }
    return level;
}


// [Wet Wealth](https://www.nutaku.net/games/wet-wealth/)
void OnLoaded() {
    LOGI("OnLoaded");
    auto AssemblyCSharp = BNM::Image("Assembly-CSharp");

    auto UserInventoryUsecase = BNM::Class("WetWealth.UserInventory", "UserInventoryUsecase",
                                           AssemblyCSharp);
    auto AddItem = UserInventoryUsecase.GetMethod("AddItem");

    auto UserCharacter = BNM::Class("WetWealth.Characters", "UserCharacter",
                                    AssemblyCSharp);
    auto AddTemptation = UserCharacter.GetMethod("AddTemptation");
    auto UserCharacterCtor = UserCharacter.GetMethod(".ctor");

    auto CityPaidLocationsUsecase = BNM::Class("WetWealth.PaidLocations",
                                               "CityPaidLocationsUsecase",
                                               AssemblyCSharp);
    auto GetLocationCurrentLevel = CityPaidLocationsUsecase.GetMethod("GetLocationCurrentLevel");


    BNM::BasicHook(AddItem, new_AddItem, old_AddItem);
    BNM::BasicHook(AddTemptation, new_AddTemptation, old_AddTemptation);
    BNM::BasicHook(UserCharacterCtor, new_UserCharacterCtor, old_UserCharacterCtor);
    BNM::BasicHook(GetLocationCurrentLevel, new_GetLocationCurrentLevel,
                   old_GetLocationCurrentLevel);
}
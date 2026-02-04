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
    std::vector<std::string> feats = {
            "Toggle:Characters",
            "Seekbar:Reward:1_20",
            "Seekbar:Temptation:1_20",
    };
    return toJobjectArray(env, feats);
}

struct Feature {
    bool characters{};
    int reward{};
    int temptation{};
};

Feature feature{false, 1, 1};

extern "C" JNIEXPORT void JNICALL
Java_com_android_support_Menu_valueChange(
        JNIEnv *env,
        jobject thiz,
        jint featIdx,
        jstring featName,
        jobject value
) {
    // featIdx: index in feature list
    switch (featIdx) {
        case 0: {
            feature.characters = toJboolean(env, value);
            break;
        }
        case 1: {
            feature.reward = toJint(env, value);
            break;
        }
        case 2: {
            feature.temptation = toJint(env, value);
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

void (*old_AddCharacterTemptation)(void *instance, void *character, int added);

void new_AddCharacterTemptation(void *instance, void *character, int added) {
    return old_AddCharacterTemptation(instance, character, added * feature.temptation);
}

void (*old_Load)(void *instance);

void new_Load(BNM::IL2CPP::Il2CppObject *instance) {
    old_Load(instance);
    if (feature.characters) {
        auto userCharacterFactory = GetField<BNM::IL2CPP::Il2CppObject *>(instance,
                                                                          "userCharacterFactory");
        auto characterStaticUsecase = GetField<BNM::IL2CPP::Il2CppObject *>(userCharacterFactory,
                                                                            "characterStaticUsecase");
        auto characters = GetProperty<BNM::IL2CPP::Il2CppObject *>(characterStaticUsecase,
                                                                   "Characters");

        auto count = GetProperty<int>(characters, "Count");
        auto getItem = GetMethod<BNM::IL2CPP::Il2CppObject *>(characters, "get_Item");
        auto AddCharacterOrExperience = GetMethod<void>(instance,
                                                        "AddCharacterOrExperience");
        for (int i = 0; i < count; ++i) {
            auto character = getItem(i);
            int id = GetProperty<int>(character, "Id");
            AddCharacterOrExperience(id, 100);
        }
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

    auto UserCharactersUsecase = BNM::Class("WetWealth.Characters", "UserCharactersUsecase",
                                            AssemblyCSharp);
    auto AddCharacterTemptation = UserCharactersUsecase.GetMethod("AddCharacterTemptation");
    auto Load = UserCharactersUsecase.GetMethod("Load");

    auto CityPaidLocationsUsecase = BNM::Class("WetWealth.PaidLocations",
                                               "CityPaidLocationsUsecase",
                                               AssemblyCSharp);
    auto GetLocationCurrentLevel = CityPaidLocationsUsecase.GetMethod("GetLocationCurrentLevel");


    BNM::BasicHook(AddItem, new_AddItem, old_AddItem);
    BNM::BasicHook(AddCharacterTemptation, new_AddCharacterTemptation, old_AddCharacterTemptation);
    BNM::BasicHook(Load, new_Load, old_Load);
    BNM::BasicHook(GetLocationCurrentLevel, new_GetLocationCurrentLevel,
                   old_GetLocationCurrentLevel);
}
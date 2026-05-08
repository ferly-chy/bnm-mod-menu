#include <jni.h>
#include <string>
#include <thread>
#include <vector>

#include "icon.h"
#include "logger.h"
#include "utils.h"
#include <BNM/Class.hpp>
#include <BNM/Field.hpp>
#include <BNM/Image.hpp>
#include <BNM/Loading.hpp>
#include <BNM/Method.hpp>
#include <BNM/Property.hpp>

extern "C" JNIEXPORT jstring JNICALL
Java_com_android_support_Menu_getTitle(JNIEnv *env, jobject) {
  return env->NewStringUTF(TITLE);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_android_support_Menu_getSubTitle(JNIEnv *env, jobject) {
  return env->NewStringUTF(SUB_TITLE);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_android_support_Menu_getStartIcon(JNIEnv *env, jobject) {
  return env->NewStringUTF(START_ICON);
}

void OnLoaded();

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  JNIEnv *env;
  vm->GetEnv((void **)&env, JNI_VERSION_1_6);
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

extern "C" JNIEXPORT void JNICALL Java_com_android_support_Menu_valueChange(
    JNIEnv *env, jobject thiz, jint featIdx, jstring featName, jint value) {
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

void (*old_AddCharacterTemptation)(void *instance, void *character, int added);

void new_AddCharacterTemptation(void *instance, void *character, int added) {
  return old_AddCharacterTemptation(instance, character,
                                    added * feature.reward);
}

void (*old_Load)(void *instance);

void new_Load(BNM::IL2CPP::Il2CppObject *instance) {
  old_Load(instance);
  if (feature.characters) {
    auto characters =
        GetMethod<BNM::IL2CPP::Il2CppObject *>(instance, "get_Characters")();
    auto count = GetMethod<int>(characters, "get_Count")();
    for (int i = 0; i < count; i++) {
      auto character =
          GetMethod<BNM::IL2CPP::Il2CppObject *>(characters, "get_Item")(i);
      GetMethod<void>(instance, "AddCharacterExp")(character, 100);
    }
  }
}

int (*old_GetLocationCurrentLevel)(void *instance, void *config);

int new_GetLocationCurrentLevel(BNM::IL2CPP::Il2CppObject *instance,
                                BNM::IL2CPP::Il2CppObject *config) {
  int level = old_GetLocationCurrentLevel(instance, config);
  if (level > 5) {
    auto staticItems =
        GetProperty<BNM::IL2CPP::Il2CppObject *>(config, "LocationKey");
    auto userInventoryUsecase =
        GetField<BNM::IL2CPP::Il2CppObject *>(instance, "userInventoryUsecase");
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

  auto UserInventoryUsecase = BNM::Class(
      "WetWealth.UserInventory", "UserInventoryUsecase", AssemblyCSharp);
  auto AddItem = UserInventoryUsecase.GetMethod("AddItem");

  auto UserCharactersUsecase = BNM::Class(
      "WetWealth.Characters", "UserCharactersUsecase", AssemblyCSharp);
  auto Load = UserCharactersUsecase.GetMethod("Load");
  auto AddCharacterTemptation =
      UserCharactersUsecase.GetMethod("AddCharacterTemptation");

  auto CityPaidLocationsUsecase = BNM::Class(
      "WetWealth.PaidLocations", "CityPaidLocationsUsecase", AssemblyCSharp);
  auto GetLocationCurrentLevel =
      CityPaidLocationsUsecase.GetMethod("GetLocationCurrentLevel");

  BNM::BasicHook(AddItem, new_AddItem, old_AddItem);
  BNM::BasicHook(Load, new_Load, old_Load);
  BNM::BasicHook(AddCharacterTemptation, new_AddCharacterTemptation,
                 old_AddCharacterTemptation);
  BNM::BasicHook(GetLocationCurrentLevel, new_GetLocationCurrentLevel,
                 old_GetLocationCurrentLevel);
}

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
      "Toggle:Currencies",
      "Toggle:Characters",
      "Seekbar:Reward:1_20",
  };
  return toJobjectArray(env, feats);
}

struct Feature {
  bool currencies{false};
  bool characters{false};
  int reward{1};
};

Feature feature{};

extern "C" JNIEXPORT void JNICALL Java_com_android_support_Menu_valueChange(
    JNIEnv *env, jobject thiz, jint featIdx, jstring featName, jint value) {
  // featIdx: index in feature list
  switch (featIdx) {
  case 0: {
    feature.currencies = value;
    break;
  }
  case 1: {
    feature.characters = value;
    break;
  }
  case 2: {
    feature.reward = value;
    break;
  }
  default:
    break;
  }
}

bool (*old_TryAdd)(void *instance, int type, int amount, void *param);

bool new_TryAdd(void *instance, int type, int amount, void *param) {
  return old_TryAdd(instance, type, amount * feature.reward, param);
}

bool (*old_Spend)(void *instance, int type, int value, void *param);

bool new_Spend(void *instance, int type, int value, void *param) {
  if (feature.currencies) {
    new_TryAdd(instance, type, value, param);
    return true;
  }
  return old_Spend(instance, type, value, param);
}

void (*old_Init)(void *instance, int level, int progress);

void new_Init(BNM::IL2CPP::Il2CppObject *instance, int level, int progress) {
  old_Init(instance, level, progress);
  if (feature.characters) {
    GetMethod<void>(instance, "AddSoul")(100);
  }
}

// [Horny Villa](https://www.nutaku.net/games/horny-villa/)
void OnLoaded() {
  LOGI("OnLoaded");
  auto AssemblyCSharp = BNM::Image("Assembly-CSharp");
  auto Currencies = BNM::Class("StripClub.Model", "Currencies", AssemblyCSharp);
  auto Spend = Currencies.GetMethod("Spend", 3);
  auto TryAdd = Currencies.GetMethod("TryAdd", 3);
  auto Promote = BNM::Class("StripClub.Model.Cards", "Promote", AssemblyCSharp);
  auto Init = Promote.GetMethod("Init");

  BNM::BasicHook(TryAdd, new_TryAdd, old_TryAdd);
  BNM::BasicHook(Spend, new_Spend, old_Spend);
  BNM::BasicHook(Init, new_Init, old_Init);
}

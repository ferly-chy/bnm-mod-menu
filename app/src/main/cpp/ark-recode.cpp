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
      "Seekbar:Attack:1_20",
      "Seekbar:Defence:1_20",
  };
  return toJobjectArray(env, feats);
}

struct Feature {
  int attack{1};
  int defence{1};
};

Feature feature{};

extern "C" JNIEXPORT void JNICALL Java_com_android_support_Menu_valueChange(
    JNIEnv *env, jobject thiz, jint featIdx, jstring featName, jint value) {
  // featIdx: index in feature list
  switch (featIdx) {
  case 0: {
    feature.attack = value;
    break;
  }
  case 1: {
    feature.defence = value;
    break;
  }
  default:
    break;
  }
}

BNM::Field<int> Camp{};

double (*old_GetSkillConditionAttack)(void *instance, void *inSource,
                                      void *inSkillProcessContext,
                                      void *inSourceSkillFunctionInfos);

double new_GetSkillConditionAttack(void *instance, void *inSource,
                                   void *inSkillProcessContext,
                                   void *inSourceSkillFunctionInfos) {
  auto ret = old_GetSkillConditionAttack(
      instance, inSource, inSkillProcessContext, inSourceSkillFunctionInfos);
  auto camp = Camp[inSource]();
  if (camp == 1) {
    ret = ret * feature.attack;
  }
  return ret;
}

double (*old_GetDefenceRate)(void *instance, void *inSource,
                             void *inSkillProcessContext,
                             void *inSourceSkillFunctionInfos);

double new_GetDefenceRate(void *instance, void *inSource,
                          void *inSkillProcessContext,
                          void *inSourceSkillFunctionInfos) {
  auto ret = old_GetDefenceRate(instance, inSource, inSkillProcessContext,
                                inSourceSkillFunctionInfos);
  auto camp = Camp[inSource]();
  if (camp == 2) {
    ret = ret * feature.defence;
  }
  return ret;
}

// [Ark Re:Code](https://www.nutaku.net/games/ark-recode/)
void OnLoaded() {
  LOGI("OnLoaded");
  auto AssemblyCSharp = BNM::Image("Assembly-CSharp");
  auto BattleRoleData = BNM::Class("Game", "BattleRoleData", AssemblyCSharp);
  Camp = BattleRoleData.GetField("Camp");
  auto BattleCalculator =
      BNM::Class("Game", "BattleCalculator", AssemblyCSharp);
  auto GetSkillConditionAttack =
      BattleCalculator.GetMethod("GetSkillConditionAttack");
  auto GetDefenceRate = BattleCalculator.GetMethod("GetDefenceRate");
  BNM::BasicHook(GetSkillConditionAttack, new_GetSkillConditionAttack,
                 old_GetSkillConditionAttack);
  BNM::BasicHook(GetDefenceRate, new_GetDefenceRate, old_GetDefenceRate);
}

#include <cstring>
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
extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_android_support_Menu_getFeatureList(JNIEnv *env, jobject thiz) {
  std::string feats[] = {"Toggle:God Mode", "Toggle:One Hit Kill",
                         "Toggle:Battle Cheat", "Toggle:Instant Skill"};
  return toJobjectArray(env, feats);
}

struct Feature {
  bool godMode{false};
  bool oneHitKill{false};
  bool battleCheat{false};
  bool instantSkill{false};
};

Feature feature{};

extern "C" JNIEXPORT void JNICALL Java_com_android_support_Menu_valueChange(
    JNIEnv *env, jobject thiz, jint featIdx, jstring featName, jint value) {
  switch (featIdx) {
  case 0:
    feature.godMode = value;
    break;
  case 1:
    feature.oneHitKill = value;
    break;
  case 2:
    feature.battleCheat = value;
    break;
  case 3:
    feature.instantSkill = value;
    break;
  default:
    break;
  }
}

// --- Hooks ---

int64_t (*old_BaseBattleUnit_Hit)(void *instance, void *attacker,
                                  int64_t damage, bool isReflectionHit,
                                  bool isFixedDamage, bool isKillDamage,
                                  bool isDamageVisible);
int64_t new_BaseBattleUnit_Hit(void *instance, void *attacker, int64_t damage,
                               bool isReflectionHit, bool isFixedDamage,
                               bool isKillDamage, bool isDamageVisible) {
  if (instance) {
    BNM::Class instanceClass((BNM::IL2CPP::Il2CppObject *)instance);
    if (instanceClass._data) {
      const char *className = instanceClass._data->name;
      if (feature.godMode && strcmp(className, "BattleHero") == 0) {
        damage = 0;
      }
      if (feature.oneHitKill && strcmp(className, "BattleEnemy") == 0) {
        damage = 999999999;
      }
    }
  }
  return old_BaseBattleUnit_Hit(instance, attacker, damage, isReflectionHit,
                                isFixedDamage, isKillDamage, isDamageVisible);
}

bool (*old_BattleManager_get_Cheat)(void *instance);
bool new_BattleManager_get_Cheat(void *instance) {
  if (feature.battleCheat)
    return true;
  return old_BattleManager_get_Cheat(instance);
}

// Instant Skill
void *(*old_BattleHero_get_HeroTableData)(void *instance);
void *new_BattleHero_get_HeroTableData(void *instance) {
  if (feature.instantSkill && instance) {
    GetMethod<void>((BNM::IL2CPP::Il2CppObject *)instance,
                    "OnlyCheatFullChargeSkill")();
  }
  return old_BattleHero_get_HeroTableData(instance);
}

void OnLoaded() {
  LOGI("OnLoaded What in HELL");
  auto AssemblyCSharp = BNM::Image("Assembly-CSharp");

  // Battle Hooks
  auto BaseBattleUnit = BNM::Class("", "BaseBattleUnit", AssemblyCSharp);
  auto BaseBattleUnit_Hit = BaseBattleUnit.GetMethod("Hit");
  BNM::BasicHook(BaseBattleUnit_Hit, new_BaseBattleUnit_Hit,
                 old_BaseBattleUnit_Hit);

  auto BattleManager = BNM::Class("", "BattleManager", AssemblyCSharp);
  auto BattleManager_get_Cheat = BattleManager.GetMethod("get_Cheat");
  BNM::BasicHook(BattleManager_get_Cheat, new_BattleManager_get_Cheat,
                 old_BattleManager_get_Cheat);

  // BattleHero Hook for Instant Skill
  auto BattleHero = BNM::Class("", "BattleHero", AssemblyCSharp);
  auto BattleHero_get_HeroTableData = BattleHero.GetMethod("get_HeroTableData");
  BNM::BasicHook(BattleHero_get_HeroTableData, new_BattleHero_get_HeroTableData,
                 old_BattleHero_get_HeroTableData);
}

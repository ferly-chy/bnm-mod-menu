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
#include "icon.h"

extern "C"
JNIEXPORT jstring JNICALL
Java_com_android_support_Menu_getTitle(JNIEnv* env, jobject) {
    return env->NewStringUTF(TITLE);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_android_support_Menu_getSubTitle(JNIEnv* env, jobject) {
    return env->NewStringUTF(SUB_TITLE);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_android_support_Menu_getStartIcon(JNIEnv* env, jobject) {
    return env->NewStringUTF(START_ICON);
}

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
            "Toggle:Currencies",
            "Toggle:Characters",
            "Seekbar:Reward:1_20"
    };
    return toJobjectArray(env, feats);
}

struct Feature {
    bool currencies{false};
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

void (*old_AddCurrencySilently)(void *instance, int type, int count);

void new_AddCurrencySilently(void *instance, int type, int count) {
    return old_AddCurrencySilently(instance, type, count * feature.reward);
}

void (*old_Spend)(void *instance, int type, int count);

void new_Spend(BNM::IL2CPP::Il2CppObject *instance, int type, int count) {
    if (feature.currencies) {
        GetMethod<void>(instance, "AddResourcesDebug")(type, count);
        return;
    }

    return old_Spend(instance, type, count);
}


void (*old_CardCharacterCtor)(void *instance, void *data, void *container);

void new_CardCharacterCtor(BNM::IL2CPP::Il2CppObject *instance, void *data, void *container) {
    old_CardCharacterCtor(instance, data, container);
    if (feature.characters) {
        GetMethod<void>(instance, "AddCopy")(100);
    }
}


// [Kinky Cats](https://www.nutaku.net/games/kinky-cats/)
void OnLoaded() {
    LOGI("OnLoaded");
    auto AssemblyCSharp = BNM::Image("Assembly-CSharp");

    auto Player = BNM::Class("Cats.Services",
                             "Player",
                             AssemblyCSharp);
    auto AddCurrencySilently = Player.GetMethod("AddCurrencySilently");
    auto Spend = Player.GetMethod("Spend");


    auto CardCharacter = BNM::Class("Cats.Characters",
                                    "CardCharacter",
                                    AssemblyCSharp);

    auto CardCharacterCtor = CardCharacter.GetMethod(".ctor");

    BNM::BasicHook(AddCurrencySilently, new_AddCurrencySilently, old_AddCurrencySilently);
    BNM::BasicHook(Spend, new_Spend, old_Spend);
    BNM::BasicHook(CardCharacterCtor, new_CardCharacterCtor, old_CardCharacterCtor);

}

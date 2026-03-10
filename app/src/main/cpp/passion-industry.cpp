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
            "Toggle:Collections",
            "Toggle:Relationship",
            "Seekbar:Reward:1_20"
    };
    return toJobjectArray(env, feats);
}

struct Feature {
    bool collections{};
    bool relationship{};
    int reward{};
};

Feature feature{false, false, 1};

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
            feature.collections = toJboolean(env, value);
            break;
        }
        case 1: {
            feature.reward = toJint(env, value);
            break;
        }
        default:
            break;
    }
}

void (*old_AddResource)(void *instance, void *resourceType, int amount, void *reason,
                        void *rewardSource);

void new_AddResource(void *instance, void *resourceType, int amount, void *reason,
                     void *rewardSource) {
    return old_AddResource(instance, resourceType, amount * feature.reward, reason, rewardSource);
}

void (*old_GiveGift)(BNM::IL2CPP::Il2CppObject *instance, void *gift);

void new_GiveGift(BNM::IL2CPP::Il2CppObject *instance, void *gift) {
    auto GiveGift = GetMethod<BNM::IL2CPP::Il2CppObject *>(instance, "GiveGift", {"expAmount"});
    return old_GiveGift(instance, gift);
}


// [Passion Industry](https://www.nutaku.net/games/passion-industry/)
void OnLoaded() {
    LOGI("OnLoaded");
    auto AssemblyCSharp = BNM::Image("Assembly-CSharp");

    auto ResourcesManager = BNM::Class("Game.Scripts.Engine.Profiles.GameResources",
                                       "ResourcesManager",
                                       AssemblyCSharp);
    auto AddResource = ResourcesManager.GetMethod("AddResource", 4);


    auto RelationshipController = BNM::Class("Game.Scripts.Relationship",
                                             "RelationshipController",
                                             AssemblyCSharp);

    auto GiveGift = RelationshipController.GetMethod("GiveGift");

    BNM::BasicHook(AddResource, new_AddResource, old_AddResource);
    BNM::BasicHook(GiveGift, new_GiveGift, old_GiveGift);

}
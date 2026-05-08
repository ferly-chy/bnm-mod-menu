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
            "Toggle:Purchase Free",
    };
    return toJobjectArray(env, feats);
}

struct Feature {
    bool purchase{false};
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
            feature.purchase = value;
            break;
        }
        default:
            break;
    }
}

bool (*old_IsProductPurchased)(void *instance, void *productId);

bool new_IsProductPurchased(void *instance, void *productId) {
    if (feature.purchase) {
        return true;
    }
    return old_IsProductPurchased(instance, productId);
}


// [Harem Fantasy](https://www.nutaku.net/games/harem-fantasy/)
void OnLoaded() {
    LOGI("OnLoaded");
    auto AssemblyCSharp = BNM::Image("Assembly-CSharp");

    auto PurchaseSaveModel = BNM::Class("HaremFantasyLib.PurchaseLib", "PurchaseSaveModel",
                                        AssemblyCSharp);
    auto IsProductPurchased = PurchaseSaveModel.GetMethod("IsProductPurchased", {"productId"});


    BNM::BasicHook(IsProductPurchased, new_IsProductPurchased, old_IsProductPurchased);
}
struct UIUBO {
    vec4 RECT;
};

layout(std140, binding = 2) uniform UIUniformBufferObject {
    UIUBO UI;
};

const float NDCX = UI.RECT.x;
const float NDCY = UI.RECT.y;
const float NDCW = UI.RECT.z;
const float NDCH = UI.RECT.w;
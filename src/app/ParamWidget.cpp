#include "global_pre.hpp"
#include "app.hpp"
#include "engine.hpp"
#include "global.hpp"
#include "global_ui.hpp"

#ifdef USE_LOG_PRINTF
extern void log_printf(const char *logData, ...);
#undef Dprintf
#define Dprintf log_printf
#else
#define Dprintf printf
#endif // USE_LOG_PRINTF


namespace rack {


json_t *ParamWidget::toJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "paramId", json_integer(paramId));

	// Infinite params should serialize to 0
	float v = (isfinite(minValue) && isfinite(maxValue)) ? value : 0.f;
	json_object_set_new(rootJ, "value", json_real(v));
	return rootJ;
}

void ParamWidget::fromJson(json_t *rootJ) {
	json_t *valueJ = json_object_get(rootJ, "value");
	if (valueJ)
   {
      float numberVal = json_number_value(valueJ);
      // Dprintf("ParamWidget::fromJson: numberVal=%f\n", numberVal);
		setValue(numberVal);
   }
}

void ParamWidget::reset() {
	// Infinite params should not be reset
	if (isfinite(minValue) && isfinite(maxValue)) {
		setValue(defaultValue);
	}
}

void ParamWidget::randomize() {
	// Infinite params should not be randomized
	if (randomizable && isfinite(minValue) && isfinite(maxValue)) {
		setValue(rescale(randomUniform(), 0.f, 1.f, minValue, maxValue));
	}
}

void ParamWidget::onMouseMove(EventMouseMove &e) {
   QuantityWidget::onMouseMove(e);
   if(!global_ui->param_info.b_lock)
      global_ui->param_info.last_param_widget = this;
}

void ParamWidget::onMouseDown(EventMouseDown &e) {
   // printf("xxx ParamWidget::onMouseDown: e.button=%d revert_val=%f\n", e.button, revert_val);
	if (e.button == 1) {
      if(INVALID_REVERT_VAL != revert_val) // during mouse drag
      {
         setValue(revert_val);
         revert_val = INVALID_REVERT_VAL;
      }
      else
      {
         reset();
      }
	}

	// if (e.button == 1) {
	// 	reset();
	// }
	e.consumed = true;
	e.target = this;
}

void ParamWidget::onChange(EventChange &e) {
	if (!module)
		return;

   // printf("xxx ParamWidget::onChange: paramId=%d value=%f this=%p smooth=%d\n", paramId, value, this, smooth);
   if(!global_ui->param_info.b_lock)
      global_ui->param_info.last_param_widget = this;

	if (smooth)
		engineSetParamSmooth(module, paramId, value);
	else
		engineSetParam(module, paramId, value);
}


} // namespace rack

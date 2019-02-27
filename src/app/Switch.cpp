#include "app/Switch.hpp"
#include "app.hpp"
#include "app/Scene.hpp"
#include "history.hpp"


namespace rack {
namespace app {


void Switch::step() {
	if (momentaryPressed) {
		momentaryPressed = false;
		// Wait another frame.
	}
	else if (momentaryReleased) {
		momentaryReleased = false;
		if (paramQuantity) {
			// Set to minimum value
			paramQuantity->setMin();
		}
	}
	ParamWidget::step();
}

void Switch::onDoubleClick(const widget::DoubleClickEvent &e) {
	// Don't reset parameter on double-click
}

void Switch::onDragStart(const widget::DragStartEvent &e) {
	if (momentary) {
		if (paramQuantity) {
			// Set to maximum value
			paramQuantity->setMax();
			momentaryPressed = true;
		}
	}
	else {
		if (paramQuantity) {
			float oldValue = paramQuantity->getValue();
			if (paramQuantity->isMax()) {
				// Reset value back to minimum
				paramQuantity->setMin();
			}
			else {
				// Increment value by 1
				paramQuantity->setValue(std::round(paramQuantity->getValue()) + 1.f);
			}

			float newValue = paramQuantity->getValue();
			if (oldValue != newValue) {
				// Push ParamChange history action
				history::ParamChange *h = new history::ParamChange;
				h->name = "move switch";
				h->moduleId = paramQuantity->module->id;
				h->paramId = paramQuantity->paramId;
				h->oldValue = oldValue;
				h->newValue = newValue;
				APP->history->push(h);
			}
		}
	}
	e.consume(this);
}

void Switch::onDragEnd(const widget::DragEndEvent &e) {
	if (momentary) {
		momentaryReleased = true;
	}
}


} // namespace app
} // namespace rack

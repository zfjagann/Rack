#include "ui/RadioButton.hpp"


namespace rack {
namespace ui {


RadioButton::RadioButton() {
	box.size.y = BND_WIDGET_HEIGHT;
}

RadioButton::~RadioButton() {
	if (quantity)
		delete quantity;
}

void RadioButton::draw(const DrawArgs &args) {
	std::string label;
	if (quantity)
		label = quantity->getLabel();
	bndRadioButton(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, label.c_str());
}

void RadioButton::onEnter(const widget::EnterEvent &e) {
	if (state != BND_ACTIVE)
		state = BND_HOVER;
	e.consume(this);
}

void RadioButton::onLeave(const widget::LeaveEvent &e) {
	if (state != BND_ACTIVE)
		state = BND_DEFAULT;
}

void RadioButton::onDragDrop(const widget::DragDropEvent &e) {
	if (e.origin == this) {
		if (state == BND_ACTIVE) {
			state = BND_HOVER;
			if (quantity)
				quantity->setMin();
		}
		else {
			state = BND_ACTIVE;
			if (quantity)
				quantity->setMax();
		}

		widget::ActionEvent eAction;
		onAction(eAction);
	}
}


} // namespace ui
} // namespace rack

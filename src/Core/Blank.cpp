#include "plugin.hpp"
#include "app.hpp"

using namespace rack;


struct BlankPanel : Widget {
	Widget *panelBorder;

	BlankPanel() {
		panelBorder = new PanelBorder;
		addChild(panelBorder);
	}

	void step() override {
		panelBorder->box.size = box.size;
		Widget::step();
	}

	void draw(const DrawArgs &args) override {
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
		nvgFillColor(args.vg, nvgRGB(0xe6, 0xe6, 0xe6));
		nvgFill(args.vg);
		Widget::draw(args);
	}
};


struct ModuleResizeHandle : Widget {
	bool right = false;
	float dragX;
	Rect originalBox;

	ModuleResizeHandle() {
		box.size = Vec(RACK_GRID_WIDTH * 1, RACK_GRID_HEIGHT);
	}

	void onButton(const widget::ButtonEvent &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			e.consume(this);
		}
	}

	void onDragStart(const widget::DragStartEvent &e) override {
		dragX = APP->scene->rackWidget->mousePos.x;
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();
		originalBox = m->box;
		e.consume(this);
	}

	void onDragMove(const widget::DragMoveEvent &e) override {
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();

		float newDragX = APP->scene->rackWidget->mousePos.x;
		float deltaX = newDragX - dragX;

		Rect newBox = originalBox;
		const float minWidth = 3 * RACK_GRID_WIDTH;
		if (right) {
			newBox.size.x += deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		}
		else {
			newBox.size.x -= deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
			newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
		}
		APP->scene->rackWidget->requestModuleBox(m, newBox);
	}

	void draw(const DrawArgs &args) override {
		for (float x = 5.0; x <= 10.0; x += 5.0) {
			nvgBeginPath(args.vg);
			const float margin = 5.0;
			nvgMoveTo(args.vg, x + 0.5, margin + 0.5);
			nvgLineTo(args.vg, x + 0.5, box.size.y - margin + 0.5);
			nvgStrokeWidth(args.vg, 1.0);
			nvgStrokeColor(args.vg, nvgRGBAf(0.5, 0.5, 0.5, 0.5));
			nvgStroke(args.vg);
		}
	}
};


struct BlankWidget : ModuleWidget {
	Widget *topRightScrew;
	Widget *bottomRightScrew;
	Widget *rightHandle;

	BlankWidget(Module *module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * 10, RACK_GRID_HEIGHT);

		panel = new BlankPanel;
		addChild(panel);

		ModuleResizeHandle *leftHandle = new ModuleResizeHandle;
		ModuleResizeHandle *rightHandle = new ModuleResizeHandle;
		rightHandle->right = true;
		this->rightHandle = rightHandle;
		addChild(leftHandle);
		addChild(rightHandle);

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		topRightScrew = createWidget<ScrewSilver>(Vec(box.size.x - 30, 0));
		bottomRightScrew = createWidget<ScrewSilver>(Vec(box.size.x - 30, 365));
		addChild(topRightScrew);
		addChild(bottomRightScrew);
	}

	void step() override {
		panel->box.size = box.size;
		topRightScrew->box.pos.x = box.size.x - 30;
		bottomRightScrew->box.pos.x = box.size.x - 30;
		if (box.size.x < RACK_GRID_WIDTH * 6) {
			topRightScrew->visible = bottomRightScrew->visible = false;
		}
		else {
			topRightScrew->visible = bottomRightScrew->visible = true;
		}
		rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
		ModuleWidget::step();
	}

	json_t *toJson() override {
		json_t *rootJ = ModuleWidget::toJson();

		// width
		json_object_set_new(rootJ, "width", json_real(box.size.x));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		ModuleWidget::fromJson(rootJ);

		// width
		json_t *widthJ = json_object_get(rootJ, "width");
		if (widthJ)
			box.size.x = json_number_value(widthJ);
	}
};


Model *modelBlank = createModel<Module, BlankWidget>("Blank");

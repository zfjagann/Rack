#include "app/PortWidget.hpp"
#include "app/Scene.hpp"
#include "window.hpp"
#include "app.hpp"
#include "history.hpp"
#include "componentlibrary.hpp"


namespace rack {
namespace app {


struct PlugLight : MultiLightWidget {
	PlugLight() {
		addBaseColor(componentlibrary::SCHEME_GREEN);
		addBaseColor(componentlibrary::SCHEME_RED);
		addBaseColor(componentlibrary::SCHEME_BLUE);
		box.size = math::Vec(8, 8);
		bgColor = componentlibrary::SCHEME_BLACK_TRANSPARENT;
	}
};


PortWidget::PortWidget() {
	plugLight = new PlugLight;
}

PortWidget::~PortWidget() {
	// plugLight is not a child and is thus owned by the PortWidget, so we need to delete it here
	delete plugLight;
	// HACK
	if (module)
		APP->scene->rackWidget->clearCablesOnPort(this);
}

void PortWidget::step() {
	if (!module)
		return;

	std::vector<float> values(3);
	for (int i = 0; i < 3; i++) {
		if (type == OUTPUT)
			values[i] = module->outputs[portId].plugLights[i].getBrightness();
		else
			values[i] = module->inputs[portId].plugLights[i].getBrightness();
	}
	plugLight->setBrightnesses(values);

	OpaqueWidget::step();
}

void PortWidget::draw(const DrawArgs &args) {
	CableWidget *cw = APP->scene->rackWidget->incompleteCable;
	if (cw) {
		// Dim the PortWidget if the active cable cannot plug into this PortWidget
		if (type == OUTPUT ? cw->outputPort : cw->inputPort)
			nvgGlobalAlpha(args.vg, 0.5);
	}
	Widget::draw(args);
}

void PortWidget::onButton(const widget::ButtonEvent &e) {
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		CableWidget *cw = APP->scene->rackWidget->getTopCable(this);
		if (cw) {
			// history::CableRemove
			history::CableRemove *h = new history::CableRemove;
			h->setCable(cw);
			APP->history->push(h);

			APP->scene->rackWidget->removeCable(cw);
			delete cw;
		}
	}
	e.consume(this);
}

void PortWidget::onEnter(const widget::EnterEvent &e) {
	hovered = true;
	e.consume(this);
}

void PortWidget::onLeave(const widget::LeaveEvent &e) {
	hovered = false;
}

void PortWidget::onDragStart(const widget::DragStartEvent &e) {
	CableWidget *cw = NULL;
	if ((APP->window->getMods() & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
		if (type == OUTPUT) {
			// Keep cable NULL. Will be created below
		}
		else {
			CableWidget *topCw = APP->scene->rackWidget->getTopCable(this);
			if (topCw) {
				cw = new CableWidget;
				cw->setOutput(topCw->outputPort);
			}
		}
	}
	else {
		// Grab cable on top of stack
		cw = APP->scene->rackWidget->getTopCable(this);

		if (cw) {
			// history::CableRemove
			history::CableRemove *h = new history::CableRemove;
			h->setCable(cw);
			APP->history->push(h);

			// Disconnect and reuse existing cable
			APP->scene->rackWidget->removeCable(cw);
			if (type == OUTPUT)
				cw->setOutput(NULL);
			else
				cw->setInput(NULL);
		}
	}

	if (!cw) {
		// Create a new cable
		cw = new CableWidget;
		if (type == OUTPUT)
			cw->setOutput(this);
		else
			cw->setInput(this);
	}

	APP->scene->rackWidget->setIncompleteCable(cw);
	e.consume(this);
}

void PortWidget::onDragEnd(const widget::DragEndEvent &e) {
	CableWidget *cw = APP->scene->rackWidget->releaseIncompleteCable();
	if (cw->isComplete()) {
		APP->scene->rackWidget->addCable(cw);

		// history::CableAdd
		history::CableAdd *h = new history::CableAdd;
		h->setCable(cw);
		APP->history->push(h);
	}
	else {
		delete cw;
	}
}

void PortWidget::onDragDrop(const widget::DragDropEvent &e) {
	// Reject ports if this is an input port and something is already plugged into it
	if (type == INPUT) {
		if (APP->scene->rackWidget->getTopCable(this))
			return;
	}

	CableWidget *cw = APP->scene->rackWidget->incompleteCable;
	if (cw) {
		cw->hoveredOutputPort = cw->hoveredInputPort = NULL;
		if (type == OUTPUT)
			cw->setOutput(this);
		else
			cw->setInput(this);
	}
}

void PortWidget::onDragEnter(const widget::DragEnterEvent &e) {
	// Reject ports if this is an input port and something is already plugged into it
	if (type == INPUT) {
		if (APP->scene->rackWidget->getTopCable(this))
			return;
	}

	CableWidget *cw = APP->scene->rackWidget->incompleteCable;
	if (cw) {
		if (type == OUTPUT)
			cw->hoveredOutputPort = this;
		else
			cw->hoveredInputPort = this;
	}
	e.consume(this);
}

void PortWidget::onDragLeave(const widget::DragLeaveEvent &e) {
	PortWidget *originPort = dynamic_cast<PortWidget*>(e.origin);
	if (!originPort)
		return;

	CableWidget *cw = APP->scene->rackWidget->incompleteCable;
	if (cw) {
		if (type == OUTPUT)
			cw->hoveredOutputPort = NULL;
		else
			cw->hoveredInputPort = NULL;
	}
}


} // namespace app
} // namespace rack

import component from "../common/component.js";
import "./sunflower.js";

component(
	`erlija`,
	class extends HTMLElement {
		sceneIn() {
			this.componentLoad.then(() => {
				const sunflower = this.shadowRoot.querySelector(`eutopia-sunflower`);
				sunflower.componentLoad.then(() => {
					document.body.setAttribute(`active`, ``);
					sunflower.setTransition(`map`, this);

					window.history.pushState(
						null,
						``,
						`${window.location.pathname}?scene=erlija`
					);
				});
			});
		}
	}
);

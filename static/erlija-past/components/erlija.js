import { registerComponent } from "../component.js";
import { updateUrlWithScene } from "../url.js";
import "./sunflower.js";
import "./storyworld-portal.js";

registerComponent(
	`erlija`,
	class extends HTMLElement {
		sceneIn() {
			this.componentLoad.then(() => {
				const sunflower = this.shadowRoot.querySelector(`emilia-sunflower`);
				sunflower.componentLoad.then(() => {
					updateUrlWithScene(this);

					document.body.setAttribute(`active`, ``);
					sunflower.setTransition(`map`, this);
				});
			});
		}
	}
);

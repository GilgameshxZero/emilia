import component from "../common/component.js";
import "./sunflower.js";

component(
	`essay`,
	class extends HTMLElement {
		constructor() {
			super();
			// Naive extend overwriting does not work.
			this.onscroll = document.querySelector(`eutopia-sunset`).onProgress;
		}

		// Begins fetching of essay; must be called before sceneIn.
		setEssay(name) {
			this.essayName = name;
			this.essayLoad = new Promise((resolve, reject) => {
				fetch(`essay/${name}.html`)
					.then((res) => {
						return res.text();
					})
					.then((text) => {
						resolve(text);
					})
					.catch((reason) => {
						reject(reason);
					});
			});
		}

		// Transition in essay scene, once it has been loaded.
		sceneIn() {
			// At this point, shadowRoot may not exist yet.
			Promise.all([
				this.essayLoad,
				// Additionally wait for load promise here, so that both fetch and load are fulfilled.
				this.componentLoad
			]).then((result) => {
				// Now, shadowRoot must exist, and so must the sunflower have been constructed (and thus have componentLoad).
				const sunflowers =
					this.shadowRoot.querySelectorAll(`eutopia-sunflower`);
				Promise.all(
					Array.from(sunflowers).map((sunflower) => {
						return sunflower.componentLoad;
					})
				).then(() => {
					// Sunflower is also ready.
					const clone = new DOMParser()
						.parseFromString(result[0], "text/html")
						.querySelector(`template`)
						.content.cloneNode(true);

					// Replace relative links.
					[`src`, `href`].forEach((attribute) => {
						clone
							.querySelectorAll(`[${attribute}^="${this.essayName}.md-assets"]`)
							.forEach((element) => {
								element.setAttribute(
									attribute,
									`essay/${element.getAttribute(attribute)}`
								);
							});
					});

					this.shadowRoot.querySelector(`article`).appendChild(clone);
					window.history.pushState(
						null,
						``,
						`${window.location.pathname}?scene=essay&essay=${this.essayName}`
					);

					// Display the essay as soon as fonts are loaded!
					document.fonts.ready.then(() => {
						sunflowers.forEach((sunflower) => {
							sunflower.setTransition(`map`, this);
						});
						document.body.setAttribute(`active`, ``);
					});
				});
			});
		}
	}
);

// Dynamic component loading.
//
// Once a component instance is constructed, componentLoad will be set. Once that promise is fulfilled, the shadowDom and component DOM is constructed (including all child components).
//
// The callback onComponentDOMContentLoaded is called when componentLoad and shadowDom exists, but child components may not have loaded yet.
//
// This function may only be called once per unique name.
export default function component(
	name,
	Component = class extends HTMLElement {}
) {
	const componentLoad = fetch(`component/${name}.html`).then((res) => {
		return res.text();
	});

	customElements.define(
		`eutopia-${name}`,
		class extends Component {
			constructor() {
				// Neither onload events nor mutation observers can detect when a component is DOMRectReadOnly. Instead, we query link/script tags and add an event listener to each: <https://stackoverflow.com/questions/45545456/event-when-web-component-has-loaded>.
				super();
				this.componentLoad = new Promise((resolve, reject) => {
					componentLoad.then((text) => {
						const clone = new DOMParser()
							.parseFromString(text, "text/html")
							.querySelector(`template`)
							.content.cloneNode(true);

						// When pending reaches 0, the component has loaded its external resources.
						let pending = 0;
						const onLoadResource = () => {
							pending--;
							if (pending === 0) {
								// Component instance ready (as soon as available fonts are loaded)!
								document.fonts.ready.then(() => {
									resolve(this);
								});
							}
						};

						// Add event listeners before loading is triggered by adding clone to the DOM.
						// TODO: Fails if no links or scripts in the component.
						clone.querySelectorAll(`link`).forEach((link) => {
							// Only links with href can be loaded.
							if (link.hasAttribute(`href`)) {
								pending++;
								link.addEventListener(`load`, onLoadResource);
							}
						});
						clone.querySelectorAll(`script`).forEach((script) => {
							// Only scripts with src can be loaded.
							if (script.hasAttribute(`src`)) {
								pending++;
								script.addEventListener(`load`, onLoadResource);
							}
						});

						// Loading is only triggered after here.
						this.attachShadow({ mode: `open` }).appendChild(clone);

						// Once shadowRoot is set, call handler.
						if (typeof this.onComponentDOMContentLoaded === `function`) {
							this.onComponentDOMContentLoaded();
						}
					});
				});
			}
		}
	);
}

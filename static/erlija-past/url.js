// Provides utility for resolving search params to a scene.
export function createSceneFromUrl() {
	let sceneName;
	if (window.location.pathname.match(/\/storyworlds\/?/g)) {
		sceneName = `erlija`;
	} else if (window.location.pathname.match(/\/snapshots\/[^\/\?#]+/g)) {
		sceneName = `snapshot`;
	} else {
		sceneName = `map`;
	}
	const scene = document.createElement(`emilia-${sceneName}`);
	document.body.appendChild(scene);
	document.querySelector(`emilia-sunset`).setAttribute(`scene`, sceneName);
	if (sceneName === `snapshot`) {
		scene.loadSnapshot(
			window.location.pathname.match(/\/snapshots\/([^\/\?#]+)/)[1]
		);
	}
	return scene;
}

// Given the current scene, then updates the URL accordingly.
export function updateUrlWithScene(scene) {
	const tagName = scene.tagName.toLowerCase();
	if (tagName == `emilia-snapshot`) {
		window.history.pushState(
			null,
			``,
			`/snapshots/${scene.snapshotName}${window.location.hash}`
		);
	} else if (tagName == `emilia-erlija`) {
		window.history.pushState(null, ``, `/storyworlds`);
	} else {
		window.history.pushState(null, ``, `/`);
	}
}

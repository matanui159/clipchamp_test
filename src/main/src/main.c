#include <stdio.h>
#include <emscripten.h>

void convert(int width, int height, int fps, int size, const void* data) {
	printf("Width: %i\nHeight: %i\n FPS: %i\nArray: %i\n", width, height, fps, size);
}

int main() {
	EM_ASM(
		var id = function(i) {
			return document.getElementById(i);
		};

		id("button").onclick = function(event) {
			id("state").innerHTML = "Reading...";
			var reader = new FileReader();
			reader.onload = function(event) {
				id("state").innerHTML = "Converting...";
				var array = event.target.result;
				ccall("convert", "number", ["number", "number", "number", "number", "array"], [
					id("width").value,
					id("height").value,
					id("fps").value,
					array.byteLength,
					array
				]);
			};
			reader.readAsArrayBuffer(id("file").files[0]);
		};
	);
	return 0;
}
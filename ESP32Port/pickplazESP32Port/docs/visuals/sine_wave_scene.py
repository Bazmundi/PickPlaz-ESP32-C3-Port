from manim import Axes, Create, Scene, Text, UP


SINTAB = [
    16, 17, 19, 21, 23, 25, 27, 30, 32, 35, 38, 41, 44, 47, 51, 54,
    58, 62, 66, 70, 74, 79, 84, 88, 93, 98, 103, 108, 114, 119, 124, 130,
    135, 141, 146, 152, 158, 163, 169, 174, 180, 185, 190, 195, 200, 205, 210, 214,
    219, 223, 227, 231, 234, 237, 240, 243, 246, 248, 250, 252, 253, 254, 255, 255,
    256, 255, 255, 254, 253, 252, 250, 248, 246, 243, 240, 237, 234, 231, 227, 223,
    219, 214, 210, 205, 200, 195, 190, 185, 180, 174, 169, 163, 158, 152, 146, 141,
    135, 130, 124, 119, 114, 108, 103, 98, 93, 88, 84, 79, 74, 70, 66, 62,
    58, 54, 51, 47, 44, 41, 38, 35, 32, 30, 27, 25, 23, 21, 19, 17,
    16, 14, 13, 11, 10, 9, 8, 7, 6, 5, 5, 4, 4, 3, 3, 2,
    2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2,
    2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 8, 9, 10, 11, 13, 14,
]


class SineTableScene(Scene):
    def construct(self) -> None:
        axes = Axes(
            x_range=[0, 255, 32],
            y_range=[0, 260, 32],
            x_length=10,
            y_length=3.5,
            tips=False,
        )
        axes_labels = axes.get_axis_labels(
            Text("index", font_size=24),
            Text("value", font_size=24),
        )
        title = Text("LED PWM Sine Table (256 samples)", font_size=32)
        title.to_edge(UP, buff=0.4)

        x_values = list(range(len(SINTAB)))
        graph = axes.plot_line_graph(
            x_values,
            SINTAB,
            add_vertex_dots=False,
        )

        self.add(title)
        self.play(Create(axes), Create(axes_labels))
        self.play(Create(graph))
        self.wait(2)

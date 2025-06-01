#ifndef PLOT_WIDGET_HPP
#define PLOT_WIDGET_HPP

#include <gtk/gtk.h>
#include <vector>

enum PlotType { PLOT_TYPE_SIGNAL, PLOT_TYPE_TIME, PLOT_TYPE_PHASOR };

#define PLOT_WIDGET_TYPE (plot_widget_get_type())
G_DECLARE_FINAL_TYPE(PlotWidget, plot_widget, PLOT, WIDGET, GtkWidget)

struct _PlotWidget {
    GtkWidget parent_instance;
    std::vector<double> original_signal;
    std::vector<double> noisy_signal;
    PlotType plot_type;
    unsigned int seed;
};

struct _PlotWidgetClass {
    GtkWidgetClass parent_class;
};

PlotWidget* plot_widget_new();
void plot_widget_set_data(PlotWidget *self, const std::vector<double>& original, const std::vector<double>& noisy, PlotType plot_type, unsigned int seed);

#endif // PLOT_WIDGET_HPP
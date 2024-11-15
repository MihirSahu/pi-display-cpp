#include "json.hpp"
//#include "resources.c"
#include "utils.cpp"
#include <curl/curl.h>
#include <gtkmm.h>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <giomm/resource.h>
#include <chrono>
#include <ctime>

//extern GResource *styles_get_resource();

using json = nlohmann::json;

struct UpdateScreenArgs {
  std::unordered_map<std::string, std::string> *data;
  std::vector<std::string> *data_order;
  Gtk::Label *label;
  int *data_index;
  Glib::RefPtr<Gtk::CssProvider> *css_provider;
  std::string *css;
};

struct UpdateTimeArgs {
  std::unordered_map<std::string, std::string> *data;
  Gtk::Label *label;
};

struct UpdateSystemMetricsArgs {
  std::unordered_map<std::string, std::string> *data;
  Gtk::Label *metrics_label;
};

gboolean update_time (gpointer args) {
  std::cout << "Updating time" << std::endl;
  UpdateTimeArgs *new_args = (UpdateTimeArgs *)args;
  std::unordered_map<std::string, std::string> *data = new_args->data;
  Gtk::Label *new_label = static_cast<Gtk::Label *>(new_args->label);

  std::time_t now = std::time(nullptr);
  std::tm* local_tm = std::localtime(&now);
  std::ostringstream oss;
  oss << std::put_time(local_tm, "%A, %B %d - %I:%M:%S %p");
  data->at("time") = oss.str();

  (*new_label).set_text(data->at("time"));

  return TRUE;
}

gboolean update_system_metrics(gpointer args) {
  std::cout << "Updating system metrics" << std::endl;
  UpdateSystemMetricsArgs *new_args = (UpdateSystemMetricsArgs *)args;
  std::unordered_map<std::string, std::string> *data = new_args->data;
  Gtk::Label *new_metrics_label = static_cast<Gtk::Label *>(new_args->metrics_label);

  std::string memory_usage = getMemoryUsage();
  std::string uptime = getUptime();
  std::string pi_temperature = getCPUTemperature();

  data->at("memory_usage") = memory_usage;
  data->at("uptime") = uptime;
  data->at("pi_temperature") = pi_temperature;
  new_metrics_label->set_text(/*"CPU - " + data->at("CPU") + " RAM - "*/ "RAM - " + data->at("memory_usage") + " Uptime - " + data->at("uptime") + " Pi Temp - " + data->at("pi_temperature"));

  return TRUE;
};

gboolean update_screen(gpointer args) {
  std::cout << "Updating main label" << std::endl;
  UpdateScreenArgs *new_args = (UpdateScreenArgs *)args;
  std::unordered_map<std::string, std::string> *data = (std::unordered_map<std::string, std::string> *)(new_args->data);
  std::vector<std::string> *data_order = (std::vector<std::string> *)(new_args->data_order);
  Gtk::Label *new_label = static_cast<Gtk::Label *>(new_args->label);
  int *data_index = (int *)(new_args->data_index);
  Glib::RefPtr<Gtk::CssProvider> *css_provider = (Glib::RefPtr<Gtk::CssProvider>*)(new_args->css_provider);
  std::string *css = (std::string*)(new_args->css);

  // Reset css
  (*css_provider)->load_from_data("");

  // Conditional css
  if (*data_index == 0) {
    std::cout << "Changing cat_fact" << std::endl;
    (*css_provider)->load_from_data(*css);
    std::string cat_fact = get_cat_fact();
    data->at("cat_fact") = cat_fact;
  }
  else if (*data_index == 1) {
    std::cout << "Changing temperature" << std::endl;
    (*css_provider)->load_from_data(*css);
    std::string temperature = get_temperature();
    data->at("temperature") = temperature + "\302\260F";
    //std::cout << "Removing update_time" << std::endl;
    //g_source_remove(std::stoul(data->at("time_timeout_id")));
    //std::cout << "Setting new label: " << (data->at(data_order->at(*data_index))) << std::endl;
  }

  new_label->set_text((data->at(data_order->at(*data_index))));
  *data_index = (*data_index + 1) % data_order->size();

  return TRUE;
}

int main(int argc, char *argv[]) {
  curl_global_init(CURL_GLOBAL_DEFAULT);

  auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

  std::unordered_map<std::string, std::string> data;
  std::vector<std::string> data_order = {"cat_fact", "temperature"};
  int data_index = 0;
  std::string css = R"(
      * {
        background-color: #27292D;
      }

      .time_label {
       font-size: xx-large;
        font-weight: 500;
        color: white;
      }

      .label1 {
        font-size: 500%;
        font-weight: 500;
        color: white;
      }

      .metrics_label {
       font-size: xx-large;
        font-weight: 500;
        color: white;
      }

      .button1 {
        font-size: large;
        font-weight: 100;
        color: white;
        padding: 10px;
      } 
    )";

  std::time_t now = std::time(nullptr);
  std::tm* local_tm = std::localtime(&now);
  std::ostringstream oss;
  oss << std::put_time(local_tm, "%a %d, %Y");

  data["time"] = oss.str();
  data["time_timeout_id"] = "";
  data["temperature"] = "Temperature";
  data["cat_fact"] = "Cat Fact";
  //data["cpu_usage"] = "";
  data["memory_usage"] = "";
  data["uptime"] = "";
  data["pi_temperature"] = "";

  Gtk::Box main_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
  main_box.set_halign(Gtk::ALIGN_FILL);
  main_box.set_valign(Gtk::ALIGN_FILL);
  main_box.set_hexpand(true);
  main_box.set_vexpand(true);
  main_box.get_style_context()->add_class("main_box");

  Gtk::Box time_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
  time_box.set_halign(Gtk::ALIGN_CENTER);
  time_box.set_valign(Gtk::ALIGN_START);
  Gtk::Label time_label = Gtk::Label(data["time"]);
  time_label.get_style_context()->add_class("time_label");
  time_box.add(time_label);

  Gtk::Box center_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
  center_box.set_halign(Gtk::ALIGN_CENTER);
  center_box.set_valign(Gtk::ALIGN_CENTER);
  Gtk::Label label = Gtk::Label(data["time"]);
  label.get_style_context()->add_class("label1");
  label.set_line_wrap(true);  // Enable line wrapping
  label.set_line_wrap_mode(Pango::WRAP_WORD);  // Wrap at word boundaries (or use Pango::WRAP_CHAR for character boundaries)
  label.set_max_width_chars(30);  // Set the maximum number of characters before wrapping
  center_box.add(label);

  Gtk::Box bottom_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
  bottom_box.set_halign(Gtk::ALIGN_CENTER);
  bottom_box.set_valign(Gtk::ALIGN_END);
  Gtk::Label metrics_label = Gtk::Label(/*"CPU - " + data["CPU"] + " RAM - "*/ "RAM - " + data["memory_usage"] + " Uptime - " + data["uptime"] + " Pi Temp - " + data["pi_temperature"]);
  metrics_label.get_style_context()->add_class("metrics_label");
  bottom_box.add(metrics_label);

  main_box.pack_start(time_box);
  main_box.pack_start(center_box);
  main_box.pack_end(bottom_box);
  //main_box.pack_start(center_box, Gtk::PACK_EXPAND_WIDGET);
  //main_box.pack_end(bottom_box, Gtk::PACK_SHRINK);

  //--overlay-light: #383B40;
  //--overlay-dark: #2D2F34;
  //--surface: #27292D;
  //--base: #1F2023;
  //--overflow: #010101;

  auto css_provider = Gtk::CssProvider::create();
  css_provider->signal_parsing_error().connect(
      [](const Glib::RefPtr<const Gtk::CssSection> &section,
         const Glib::Error &error) {
        std::cerr << "CSS parsing error: " << error.what() << std::endl;
      });
  css_provider->load_from_data(css);
  Gtk::StyleContext::add_provider_for_screen(
      Gdk::Screen::get_default(), css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  UpdateScreenArgs screenArgs = {&data, &data_order, &label, &data_index, &css_provider, &css};
  UpdateTimeArgs timeArgs = {&data, &time_label};
  UpdateSystemMetricsArgs systemMetricsArgs = {&data, &metrics_label};
  guint update_screen_timeout_id = g_timeout_add_seconds(5, &update_screen, &screenArgs);
  guint update_system_metrics_timeout_id = g_timeout_add_seconds(5, &update_system_metrics, &systemMetricsArgs);
  guint update_time_timeout_id = g_timeout_add_seconds(1, &update_time, &timeArgs);

  Gtk::Window window;
  window.set_default_size(400, 300);
  window.set_title("Pi Dashboard");
  window.add(main_box);
  main_box.show_all();
  center_box.show_all();
  bottom_box.show_all();

  window.fullscreen();

  return app->run(window);
}

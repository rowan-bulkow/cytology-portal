using Microsoft.Owin;
using Owin;

[assembly: OwinStartupAttribute(typeof(cytology_portal.Startup))]
namespace cytology_portal
{
    public partial class Startup
    {
        public void Configuration(IAppBuilder app)
        {
            ConfigureAuth(app);
        }
    }
}

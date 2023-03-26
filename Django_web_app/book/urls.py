from django.contrib.staticfiles.urls import staticfiles_urlpatterns
from django.urls import path

from book import views
from book.models import Transaction
from currency import views as currency_views

home_list_view = views.HomeListView.as_view(
    queryset = Transaction.objects.order_by('-DateTime')[:100],  # :5 limits the results to the five most recent
    context_object_name = 'transaction_list',
    template_name = 'book/home.html',
)

urlpatterns = [
    path('', home_list_view, name='home'),
    path('about/', views.about, name='about'),
    path('contact/', views.contact, name='contact'),
    path('add_transaction/', views.addTransaction, name='add_transaction'),
    path('delete_transaction/<int:transaction_id>', views.deleteTransaction, name='delete_transaction'),
    path('update_transaction/<int:transaction_id>', views.UpdateTransaction, name='update_transaction'),
    
    path('update_currency/', currency_views.UpdateCurrency, name='update_currency'),
]

urlpatterns += staticfiles_urlpatterns()